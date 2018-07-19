要理解TSRM,首先需要知道Thread-Specific Data.

# Thread-Specific Data

1. 告诉pthread,我将要给进程的每个thread分配一块内存,但现在不分配,你给我一个这块内存的标记名称(pthread\_key\_t),并记下来销毁的办法(destructor),在进程结束的时候销毁这块内存.

        int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));

2. 当需要用到这块内存时(pthread\_key\_t),先get下,如果还没有分配,则分配,并把内存地址告诉pthread,让它记下来,这样下次get时就直接拿来用了.

        void *pthread_getspecific(pthread_key_t key);
        int pthread_setspecific(pthread_key_t key, const void *value);

猜想: pthread那里应该是一个key-addr的map,每个pthread都有这个map,key是一样的,只不过addr有的thread为空,有的不为空.

操作系统限制一个进程可利用的key的数量不能超过指定值.因此一般一个library只用一个key.在library内部再实现支持多个key的逻辑.TSRM就是如此.

# TSRM

TSRM维护一个hashtable(tsrm\_tls\_table),hashtable里存储着每个thread的信息.

为什么要用hashtable呢?

猜想原因:

pthread\_key\_create()时将destructor设成null了,为了有办法free掉分配的内存,需要将每个thread分配的内存信息记录下来.这样最终free时遍历这个hashtable就可以了.

那为什么pthread\_key\_create要将destructor设成null呢?为什么要自己free内存信息,而不是让pthread来做这个事情呢?

猜想原因:

php要支持多个操作系统,支持多种thread实现,可能不是所有的thread实现都有pthread这样的机制吧.

    struct _tsrm_tls_entry {
        void **storage;
        int count;
        THREAD_T thread_id;
        tsrm_tls_entry *next;
    };
    static tsrm_tls_entry   **tsrm_tls_table=NULL

tsrm\_tls\_entry里记录了thread_id,这个thread当前已经分配了多少块内存,以及内存地址列表.

那怎么识别每块内存里的内容呢?

内存地址列表的index对应着resouce_type,那里记录着这块内存有多大,怎么初始化,怎么free.

    typedef struct {
        size_t size;
        ts_allocate_ctor ctor;
        ts_allocate_dtor dtor;
        int done;
    } tsrm_resource_type;

# TSRM API

    int tsrm_startup(int expected_threads, int expected_resources, int debug_level, char *debug_filename);

按照expected\_threads初始化hashtable(tsrm\_tls\_table),按照expected\_resources初始化resource\_types\_table.

    ts_rsrc_id ts_allocate_id(ts_rsrc_id *rsrc_id, size_t size, ts_allocate_ctor ctor, ts_allocate_dtor dtor);

向resource\_types\_table中添加一个新的resouce,并拿到resouce\_id. 同时会给已存在的每个thread分配该resouce需要的内存并初始化.这样当thread需要这个resouce时,就已经ready了.

    void *ts_resource_ex(ts_rsrc_id id, THREAD_T *th_id);

获取指定的resouce,tsrm\_startup时只是分配了hashtable,并没有初始化已有thread的tsrm\_tls\_entry,获取resource时,如果发现指定的thread还没有对应的tsrm\_tls\_entry,则分配之并初始化resource\_types\_table里的resource.

如果一个thread获取自己的resouce,这里就用到了pthread\_getspecific(),pthread\_setspecific(),如果获取其它thread的resouce,就是查hashtable了.

    void tsrm_shutdown(void);

free掉hashtable,free掉resouce\_types\_table.
