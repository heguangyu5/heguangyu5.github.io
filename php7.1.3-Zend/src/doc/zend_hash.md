# 一个重要的数学运算

    a = power of 2;
    uint | -a 的结果范围在 -a 到 -1

        #include <stdio.h>
        #include <stdint.h>

        int main(int argc, char *argv[])
        {
            if (argc != 2) {
                printf("usage: %s -4\n", argv[0]);
                return 1;
            }

            int32_t n = atoi(argv[1]);
            if (n >= 0) {
                printf("usage: %s -4\n", argv[0]);
                return 1;
            }

            uint32_t n2 = (-n) * 2;
            int i;
            for (i = 0; i < n2; i++) {
                printf("%d | %d = %d\n", i, n, i | n);
            }

            return 0;
        }
        
        gcc a.c && ./a.out -4
        0 | -4 = -4
        1 | -4 = -3
        2 | -4 = -2
        3 | -4 = -1
        4 | -4 = -4
        5 | -4 = -3
        6 | -4 = -2
        7 | -4 = -1

# php7数组的实现

    为了保证数组遍历的顺序和插入顺序一致,php7数组元素(zval)就按普通的C语言数组保存
    
    | zval | zval | zval | zval | zval | zval | zval | zval |
    |  0   |  1   |  2   |  3   |  4   |  5   |  6   |  7   |
    
    从数组里删除元素不是真的删除了,而是将其设为UNDEF(zval.ui.type = 0 就表示是 UNDEF).
    
    | zval | UNDEF| zval | zval | zval | zval | zval | zval |
    |  0   |  1   |  2   |  3   |  4   |  5   |  6   |  7   |
    
    显然遍历的时候要做下判断
        for () {
            if (zval IS UNDEF) {
                continue;
            }
        }
    
    插入时为了快速知道数组的末尾,用 nNumUsed 记录下.
    为了快速知道数组里元素的个数,用 nNumOfElements 记录下.
    
    怎么实现hash table的快速访问呢?
    把zval的hash key与C数组的index做个map.这个map保存在C数组的前边.
    
    | c index  | c index  | c index  | c index  | zval | zval | zval | zval | zval | zval | zval | zval |
    | hash key | hash key | hash key | hash key |  0   |  1   |  2   |  3   |  4   |  5   |  6   |  7   |

    hash key的计算就按上边说的数学运算来: hash(zval) | -a
    比如数组大小为4,那么结构如下
    
    | c index  | c index  | c index  | c index  | UNDEF| UNDEF| UNDEF| UNDEF|
    |    -4    |    -3    |    -2    |    -1    |  0   |  1   |  2   |  3   |
    
    插入第1个元素, c index = 0.
    hash(zval) | -4 假设等于 -3. 那就把 -3 的 c index 设为 0
    
    | c index  |     0    | c index  | c index  | zval | UNDEF| UNDEF| UNDEF|
    |    -4    |    -3    |    -2    |    -1    |  0   |  1   |  2   |  3   |
    
    怎么解决hash冲突呢?
    插入第2个元素, c index = 1
    hash(zval) | -4 假设还等于 -3. 那就把第2个元素的 zval.u2.next = 0, 然后 -3 处的 c index = 1
    结果如下:
    
    | c index  |     1    | c index  | c index  | zval | zval | UNDEF| UNDEF|
    |    -4    |    -3    |    -2    |    -1    |  0   |  1   |  2   |  3   |
    
    查找时 hash(zval) | -4 = -3, 从 -3 处取得 c index = 1, 比较1的zval,如果不等,根据zval.u2.next找到下一个c index.
    怎么判断这个链表结束呢? zval.u2.next = HT\_INVALID\_IDX = (uint32_t)-1 表示结束.
    hash table的大小会限制不会达到 (uint32_t)-1.
    这样的话,就要求每个zval.u2.next默认就是 HT\_INVALID\_IDX. 这怎么做到呢?
    让 c index 默认等于 HT\_INVALID\_IDX, 然后在插入时始终上 zval.u2.next = c index 就好了.


# 读一下源码

        void _zend_hash_init(HashTable *ht,  uint32_t nSize,  dtor_func_t pDestructor,  zend_bool persistent)
        {
            GC_REFCOUNT(ht)      = 1;
            GC_TYPE_INFO(ht)     = IS_ARRAY;
            ht->u.flags          = (persistent ? HASH_FLAG_PERSISTENT : 0)
                                    | HASH_FLAG_APPLY_PROTECTION
                                    | HASH_FLAG_STATIC_KEYS;
            ht->nTableMask       = HT_MIN_MASK // = -2;
            HT_SET_DATA_ADDR(ht, &uninitialized_bucket); // [ HT_INVALID_IDX, HT_INVALID_IDX ]
            ht->nNumUsed         = 0;
            ht->nNumOfElements   = 0;
            ht->nInternalPointer = HT_INVALID_IDX;
            ht->pDestructor      = pDestructor;
            ht->nTableSize       = zend_hash_check_size(nSize); // 把nSize调整到power of 2
        }
        // _zend_hash_init()做了最简的初始化. arData指向了uninitialized_bucket并修正了nTableSize
        
        zend_hash_real_init(HashTable *ht, zend_bool packed) => zend_hash_real_init_ex(ht, packed);
            // 如果不是packed array
                // 修正nTableMask
                ht->nTableMask = -(ht->nTableSize);
                // 修正arData
                HT_SET_DATA_ADDR(ht, pemalloc(HT_SIZE(ht), (ht)->u.flags & HASH_FLAG_PERSISTENT));
                    // HT_SIZE(ht) = HT_DATA_SIZE(nTableSize) + HT_HASH_SIZE(nTableMask)
                    //             = nTableSize * sizeof(Bucket) + (-nTableMask) * sizeof(uint32_t)
                // 标记已初始化
                ht->u.flags |= HASH_FLAG_INITIALIZED;
                // 将arData左边的c index部分初始化为HT_INVALID_IDX
            // 如果是packed array
                // 注意这里没有修正nTableMask
                // 修正arData
                HT_SET_DATA_ADDR(ht, pemalloc(HT_SIZE(ht), (ht)->u.flags & HASH_FLAG_PERSISTENT));
                // 标记已初始化,是packed array
                ht->u.flags |= HASH_FLAG_INITIALIZED | HASH_FLAG_PACKED;
                // 将arData左边的两个c index设为HT_INVALID_IDX
        
        zval* _zend_hash_add(HashTable *ht, zend_string *key, zval *pData)
        {
            return _zend_hash_add_or_update_i(ht, key, pData, HASH_ADD);
        }
        zval *_zend_hash_add_or_update_i(HashTable *ht, zend_string *key, zval *pData, uint32_t flag)
        {
            // 先检测下ht有没有初始化,如果没有,初始化它,然后add_to_hash
            if (!(ht->u.flags & HASH_FLAG_INITIALIZED)) {
                zend_hash_real_init(ht, 0); // packed = 0, 因为key是zend_string
                goto add_to_hash;
            } else if (ht->u.flags & HASH_FLAG_PACKED) {
                // 向packed array添加string key
                zend_hash_packed_to_hash(ht);
            } else if (flag & HASH_ADD_NEW == 0) {
                // 我们是HASH_ADD,不是HASH_ADD_NEW,所以会执行下边的逻辑
                p = zend_hash_find_bucket(ht, key);
                // hash(key) | nTableMask 找到 c index 的位置,然后得到c index的值
                // 如果此处是HT_INVALID_IDX, 看ht是否需要expand,然后add_to_hash
                // 如果不是HT_INVALID_IDX,说明hash冲突了,遍历链表找key,如果找到了,返回bucket,否则返回null
                if (p) {
                }
            }
            
            ZEND_HASH_IF_FULL_DO_RESIZE(ht);
                => zend_hash_do_resize(ht);
                    // 如果数组中有很多unset的,那就不resize,而是直接rehash
                    // 否则,新分配一个hash table,大小是原来的2倍,然后rehash
                    zend_hash_rehash(ht);
            
            add_to_hash:
                idx = ht->nNumUsed++;
                ht->nNumOfElements++;
                if (ht->nInternalPointer == HT_INVALID_IDX) {
                    ht->nInternalPointer = idx;
                }
                zend_hash_iterators_update(ht, HT_INVALID_IDX, idx);
                p = ht->arData + idx;
                p->key = key;
                if (!ZSTR_IS_INTERNED(key)) {
                    zend_string_addref(key);
                    ht->u.flags &= ~HASH_FLAG_STATIC_KEYS;
                    zend_string_hash_val(key);
                }
                p->h = ZSTR_H(key);
                ZVAL_COPY_VALUE(&->val, pData);
                nIndex = h | ht->nTableMask;
                Z_NEXT(p->val) = HT_HASH(ht, nIndex);
                HT_HASH(ht, nIndex) = idx;

                return &p->val;
        }
        
        zval* _zend_hash_update(HashTable *ht, zend_string *key, zval *pData)
        {
            return _zend_hash_add_or_update_i(ht, key, pData, HASH_UPDATE);
        }
        zval* _zend_hash_update_ind(HashTable *ht, zend_string *key, zval *pData)
        {
            return _zend_hash_add_or_update_i(ht, key, pData, HASH_UPDATE | HASH_UPDATE_INDIRECT);
        }
        zval* _zend_hash_add_new(HashTable *ht, zend_string *key, zval *pData)
        {
            return _zend_hash_add_or_update_i(ht, key, pData, HASH_ADD_NEW);
        }
