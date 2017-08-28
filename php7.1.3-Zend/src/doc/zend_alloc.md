zend将分配内存的大小分为3类:

1. <= 3K        small
2. <= 2044K     large
3. >  2044K     huge

**huge**

huge类的内存块直接就是mmap, mmap的结果保存在struct zend\_mm\_huge\_list里.

        struct zend_mm_huge_list {
            void *ptr;
            size_t size;
            zend_mm_huge_list *next;
        };
        
        struct zend_mm_heap {
            // ...
            zend_mm_huge_list *huge_list;
            // ...
        };

显然多个huge内存块形成了一个单链表.

从heap->huge_list开始,就能按分配时间倒序遍历所有的huge内存块了.

**chunk**

large和small基于chunk,所以先看看chunk.

一个chunk是2M,一个page是4K,所以一个chunk有512个page.

其中保留第一个page做自身管理用.

        struct zend_mm_chunk {
            zend_mm_heap *heap;
            zend_mm_chunk *next;
            zend_mm_chunk *prev;
            uint32_t free_pages;
            uint32_t free_tail;
            uint32_t num;
            char reserve[64 - (sizeof(void *) * 3 + sizeof(int) * 3)];
            uint64_t free_map[8];
            uint32_t map[512];
        };

        struct zend_mm_heap {
            // ...
            zend_mm_chunk *main_chunk;
            // ...
        };

zend\_mm\_chunk这个struct大小大于2K. chunk保留的第一个page就强制转换成这个struct了.

php启动时,zend\_mm\_init()分配了一个chunk,并将此chunk做为heap的main_chunk.

zend\_mm\_heap的main_chunk和后来又分配的chunk形成了一个环形双向链表.

        free_pages = 511;   // 这个chunk当前共有511个free page
        free_tail = 1;      // 从第1个page开始后边的page都是free的
        num = chunk->prev->num + 1; // main_chunk->num = 0; 后边每分配一个chunk对应的num就加1
        free_map 共有64*8=512bits,第1个字节的第1个bit已设为1表示第1个page已经用了.


**large**

一个chunk申请下来后,由于保留了1个page,所以最多只能有511个page=2044K可用了.

所有large类的内存块大小都小于或等于2044K.

这个chunk再向外分配内存时,根据free\_pages的数量可以快速判断这个chunk是否能用.如果能找到可用的pages,返回.如果不能,去找链表里的下一个chunk.如果找到main\_chunk(不包括main\_chunk)还没找到,那就新申请一个chunk.

**small**

对于small类的内存块,zend事先计划好对于某个大小的一次申请多少个page.

当然这些page从是chunk里分配的.

比如大小为3K的内存块,一次申请3个page,共12K内存.将这12K内存分成4个3K的,并强制转换成zend\_mm\_free\_slot,串成一个单链表.并将第2个内存块的地址记到heap的free_slot里. 这样下次再申请这个大小的内存块时,直接就返回了.

        struct _zend_mm_free_slot {
            zend_mm_free_slot *next_free_slot;
        };
        
        struct zend_mm_heap {
            // ...
            zend_mm_free_slot *free_slot[30];
            // ...
        };

**总结一下**

1. zend\_mm\_init() 申请了一个chunk做main_chunk,并取这个chunk第1个page里的一块内存做为heap. 外部如果想用的话,可以通过调用以下两个函数间接调用zend\_mm\_init.

        zend_mm_heap *zend_mm_startup(void);
        void start_memory_manager(void);

2. small/large/huge内存块的申请是通过 zend\_mm\_alloc\_heap() 间接申请的. 而  zend\_mm\_alloc\_heap() 也不能在外部直接调用.外部调用对应不同的init方法也分两种:

        // init: zend_mm_heap *zend_mm_startup(void);
        void* _zend_mm_alloc(zend_mm_heap *heap, size_t size);
        // init: void start_memory_manager(void);
        void* _emalloc(size_t size);

3. 完整的API列表:

        // init: zend_mm_heap *zend_mm_startup(void);
        void* _zend_mm_alloc(zend_mm_heap *heap, size_t size);
        void  _zend_mm_free(zend_mm_heap *heap, void *ptr);
        void* _zend_mm_realloc(zend_mm_heap *heap, void *ptr, size_t size);
        void* _zend_mm_realloc2(zend_mm_heap *heap, void *ptr, size_t size, size_t copy_size);
        size_t _zend_mm_block_size(zend_mm_heap *heap, void *ptr);
        // init: void start_memory_manager(void);
        void* _emalloc(size_t size);
        void  _efree(void *ptr);
        void* _erealloc(void *ptr, size_t size);
        void* _erealloc2(void *ptr, size_t size, size_t copy_size);
        size_t _zend_mem_block_size(void *ptr);

4. 怎么直接使用malloc函数?

        pemalloc(size, persistent);
        pefree(ptr, persistent);
        // 有什么区别?
        // emalloc()分配的内存随着memory_manager shutdown就释放掉了
        // pemalloc()分配的内存不会释放,直到进程退出

5. 这么多选择,平时写代码该怎么用?

        看zend_hash.c的实现,较多的用pemalloc和emalloc.
