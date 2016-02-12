#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;

enum zone_watermarks {
	WMARK_MIN,
	WMARK_LOW,
	WMARK_HIGH,
	NR_WMARK
};

enum zone_type {
	ZONE_DMA,
	ZONE_DMA32,
	ZONE_NORMAL,
	ZONE_MOVABLE,
	MAX_NR_ZONES
};

enum zone_stat_item {
	/* First 128 byte cacheline (assuming 64 bit words) */
	NR_FREE_PAGES,
	NR_LRU_BASE,
	NR_INACTIVE_ANON = NR_LRU_BASE, /* must match order of LRU_[IN]ACTIVE */
	NR_ACTIVE_ANON,		/*  "     "     "   "       "         */
	NR_INACTIVE_FILE,	/*  "     "     "   "       "         */
	NR_ACTIVE_FILE,		/*  "     "     "   "       "         */
	NR_UNEVICTABLE,		/*  "     "     "   "       "         */
	NR_MLOCK,		/* mlock()ed pages found and moved off LRU */
	NR_ANON_PAGES,	/* Mapped anonymous pages */
	NR_FILE_MAPPED,	/* pagecache pages mapped into pagetables.
			   only modified from process context */
	NR_FILE_PAGES,
	NR_FILE_DIRTY,
	NR_WRITEBACK,
	NR_SLAB_RECLAIMABLE,
	NR_SLAB_UNRECLAIMABLE,
	NR_PAGETABLE,		/* used for pagetables */
	NR_KERNEL_STACK,
	/* Second 128 byte cacheline */
	NR_UNSTABLE_NFS,	/* NFS unstable pages */
	NR_BOUNCE,
	NR_VMSCAN_WRITE,
	NR_WRITEBACK_TEMP,	/* Writeback using temporary buffers */
	NR_ISOLATED_ANON,	/* Temporary isolated pages from anon lru */
	NR_ISOLATED_FILE,	/* Temporary isolated pages from file lru */
	NR_SHMEM,		/* shmem pages (included tmpfs/GEM pages) */
	NR_DIRTIED,		/* page dirtyings since bootup */
	NR_WRITTEN,		/* page writings since bootup */
	NUMA_HIT,		/* allocated in intended node */
	NUMA_MISS,		/* allocated in non intended node */
	NUMA_FOREIGN,		/* was intended here, hit elsewhere */
	NUMA_INTERLEAVE_HIT,	/* interleaver preferred this zone */
	NUMA_LOCAL,		/* allocation from local node */
	NUMA_OTHER,		/* allocation from other node */
	NR_ANON_TRANSPARENT_HUGEPAGES,
	NR_VM_ZONE_STAT_ITEMS
};

#define LRU_BASE 0
#define LRU_ACTIVE 1
#define LRU_FILE 2

enum lru_list {
	LRU_INACTIVE_ANON = LRU_BASE,
	LRU_ACTIVE_ANON = LRU_BASE + LRU_ACTIVE,
	LRU_INACTIVE_FILE = LRU_BASE + LRU_FILE,
	LRU_ACTIVE_FILE = LRU_BASE + LRU_FILE + LRU_ACTIVE,
	LRU_UNEVICTABLE,
	NR_LRU_LISTS
};

struct list_head {
    struct list_head *next, *prev;
};

typedef struct spinlock {
	unsigned int slock;
} spinlock_t;

typedef struct {
    unsigned sequence;
    spinlock_t lock;
} seqlock_t;

#define __aligned(x) __attribute__((aligned(x)))
typedef struct {
    u64 __aligned(8) counter;
} atomic64_t;

typedef atomic64_t atomic_long_t;

struct __wait_queue_head {
    spinlock_t lock;
    struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_PER_BYTE       8
#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#define DECLARE_BITMAP(name,bits) \
   unsigned long name[BITS_TO_LONGS(bits)]

#define __percpu // 不是很确定 @see include/linux/compiler.h#0041

#define MIGRATE_UNMOVABLE     0
#define MIGRATE_RECLAIMABLE   1
#define MIGRATE_MOVABLE       2
#define MIGRATE_PCPTYPES      3 /* the number of types on the pcp lists */
#define MIGRATE_RESERVE       3
#define MIGRATE_ISOLATE       4 /* can't allocate from here */
#define MIGRATE_TYPES         5

#define MAX_ORDER 11

#define MAX_NUMNODES 64
#define MAX_ZONES_PER_ZONELIST (MAX_NUMNODES * MAX_NR_ZONES)

struct per_cpu_pages {
	int count;
	int high;
	int batch;
	struct list_head lists[MIGRATE_PCPTYPES];
};

struct per_cpu_pageset {
	struct per_cpu_pages pcp;
	s8 expire;
	s8 stat_threshold;
	s8 vm_stat_diff[NR_VM_ZONE_STAT_ITEMS];
};

struct free_area {
	struct list_head	free_list[MIGRATE_TYPES];
	unsigned long		nr_free;
};

#define ____cacheline_internodealigned_in_smp \
     __attribute__((__aligned__(1 << 7)))

struct zone_padding {
	char x[0];
} ____cacheline_internodealigned_in_smp;
#define ZONE_PADDING(name)	struct zone_padding name;

struct zone_reclaim_stat {
	unsigned long		recent_rotated[2];
	unsigned long		recent_scanned[2];
	unsigned long		nr_saved_scan[NR_LRU_LISTS];
};

struct zone {
	/* Fields commonly accessed by the page allocator */

	/* zone watermarks, access with *_wmark_pages(zone) macros */
	unsigned long watermark[NR_WMARK];

	/*
	 * When free pages are below this point, additional steps are taken
	 * when reading the number of free pages to avoid per-cpu counter
	 * drift allowing watermarks to be breached
	 */
	unsigned long percpu_drift_mark;

	/*
	 * We don't know if the memory that we're going to allocate will be freeable
	 * or/and it will be released eventually, so to avoid totally wasting several
	 * GB of ram we must reserve some of the lower zone memory (otherwise we risk
	 * to run OOM on the lower zones despite there's tons of freeable ram
	 * on the higher zones). This array is recalculated at runtime if the
	 * sysctl_lowmem_reserve_ratio sysctl changes.
	 */
	unsigned long		lowmem_reserve[MAX_NR_ZONES];

	int node;
	/*
	 * zone reclaim becomes active if more unmapped pages exist.
	 */
	unsigned long		min_unmapped_pages;
	unsigned long		min_slab_pages;

	struct per_cpu_pageset __percpu *pageset;
	/*
	 * free areas of different sizes
	 */
	spinlock_t		lock;
	int                     all_unreclaimable; /* All pages pinned */

	/* see spanned/present_pages for more description */
	seqlock_t		span_seqlock;

	struct free_area	free_area[MAX_ORDER];

	/*
	 * On compaction failure, 1<<compact_defer_shift compactions
	 * are skipped before trying again. The number attempted since
	 * last failure is tracked with compact_considered.
	 */
	unsigned int		compact_considered;
	unsigned int		compact_defer_shift;


	ZONE_PADDING(_pad1_)

	/* Fields commonly accessed by the page reclaim scanner */
	spinlock_t		lru_lock;
	struct zone_lru {
		struct list_head list;
	} lru[NR_LRU_LISTS];

	struct zone_reclaim_stat reclaim_stat;

	unsigned long		pages_scanned;	   /* since last reclaim */
	unsigned long		flags;		   /* zone flags, see below */

	/* Zone statistics */
	atomic_long_t		vm_stat[NR_VM_ZONE_STAT_ITEMS];

	/*
	 * The target ratio of ACTIVE_ANON to INACTIVE_ANON pages on
	 * this zone's LRU.  Maintained by the pageout code.
	 */
	unsigned int inactive_ratio;


	ZONE_PADDING(_pad2_)
	/* Rarely used or read-mostly fields */

	/*
	 * wait_table		-- the array holding the hash table
	 * wait_table_hash_nr_entries	-- the size of the hash table array
	 * wait_table_bits	-- wait_table_size == (1 << wait_table_bits)
	 *
	 * The purpose of all these is to keep track of the people
	 * waiting for a page to become available and make them
	 * runnable again when possible. The trouble is that this
	 * consumes a lot of space, especially when so few things
	 * wait on pages at a given time. So instead of using
	 * per-page waitqueues, we use a waitqueue hash table.
	 *
	 * The bucket discipline is to sleep on the same queue when
	 * colliding and wake all in that wait queue when removing.
	 * When something wakes, it must check to be sure its page is
	 * truly available, a la thundering herd. The cost of a
	 * collision is great, but given the expected load of the
	 * table, they should be so rare as to be outweighed by the
	 * benefits from the saved space.
	 *
	 * __wait_on_page_locked() and unlock_page() in mm/filemap.c, are the
	 * primary users of these fields, and in mm/page_alloc.c
	 * free_area_init_core() performs the initialization of them.
	 */
	wait_queue_head_t	* wait_table;
	unsigned long		wait_table_hash_nr_entries;
	unsigned long		wait_table_bits;

	/*
	 * Discontig memory support fields.
	 */
	struct pglist_data	*zone_pgdat;
	/* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT */
	unsigned long		zone_start_pfn;

	/*
	 * zone_start_pfn, spanned_pages and present_pages are all
	 * protected by span_seqlock.  It is a seqlock because it has
	 * to be read outside of zone->lock, and it is done in the main
	 * allocator path.  But, it is written quite infrequently.
	 *
	 * The lock is declared along with zone->lock because it is
	 * frequently read in proximity to zone->lock.  It's good to
	 * give them a chance of being in the same cacheline.
	 */
	unsigned long		spanned_pages;	/* total size, including holes */
	unsigned long		present_pages;	/* amount of memory (excluding holes) */

	/*
	 * rarely used fields:
	 */
	const char		*name;
} ____cacheline_internodealigned_in_smp;

struct zoneref {
    struct zone *zone;  /* Pointer to actual zone */
    int zone_idx;       /* zone_idx(zoneref->zone) */
};

#define MAX_ZONELISTS 2
struct zonelist_cache {
    unsigned short z_to_n[MAX_ZONES_PER_ZONELIST];      /* zone->nid */
    DECLARE_BITMAP(fullzones, MAX_ZONES_PER_ZONELIST);  /* zone full? */
    unsigned long last_full_zap;
};

struct zonelist {
	struct zonelist_cache *zlcache_ptr;		     // NULL or &zlcache
	struct zoneref _zonerefs[MAX_ZONES_PER_ZONELIST + 1];
	struct zonelist_cache zlcache;
};

typedef struct pglist_data {
	struct zone node_zones[MAX_NR_ZONES];
	struct zonelist node_zonelists[MAX_ZONELISTS];
	int nr_zones;

	spinlock_t node_size_lock;

	unsigned long node_start_pfn;
	unsigned long node_present_pages; /* total number of physical pages */
	unsigned long node_spanned_pages; /* total size of physical page range, including holes */
	int node_id;
	wait_queue_head_t kswapd_wait;
	void *kswapd; // struct task_struct *kswapd;
	int kswapd_max_order;
	enum zone_type classzone_idx;
} pg_data_t;


typedef struct {
    int counter;
} atomic_t;

struct page {
	unsigned long flags;
	atomic_t _count;
	union {
		atomic_t _mapcount;
		struct {
			u16 inuse;
			u16 objects;
		};
	};
	union {
	    struct {
		    unsigned long private;
		    struct address_space *mapping;
	    };

	    spinlock_t ptl;

	    struct kmem_cache *slab;
	    struct page *first_page;
	};
	union {
		u64 index;
		void *freelist;
	};
	struct list_head lru;
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct pglist_data);
    printf("sizeof(struct pglist_data) = %#x\n", size);

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/node_data.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct pglist_data *node_data;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    node_data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (node_data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("node_data.node_id               = %d\n",   node_data->node_id);
    printf("node_data.node_start_pfn        = %#lx\n", node_data->node_start_pfn);
    printf("node_data.node_spanned_pages    = %#lx\n", node_data->node_spanned_pages);
    printf("node_data.node_present_pages    = %#lx\n", node_data->node_present_pages);
    printf("node_data.nr_zones              = %#x\n",  node_data->nr_zones);

    int fd2;
    struct page *pages = NULL;
    int pagesSize = 0x200000;
#define PAGE_MASK 0xffffea0000000000
    if (argc == 3) {
        fd2 = open(argv[2], O_RDONLY);
        if (fd2 == -1) {
            perror("open");
            return 1;
        }
        pages = mmap(NULL, pagesSize, PROT_READ, MAP_SHARED, fd2, 0);
        if (pages == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
    }

    int i, j, k;
    struct zone *zone;
    struct free_area *free_area;
    uint64_t page_lru_prev, page_lru_next, page_lru_head, page_lru_current;
    int pfn;
    struct page *page;
    for (i = 0; i < MAX_NR_ZONES; i++) {
        zone = &node_data->node_zones[i];
        printf("zone %d", i);
        if ((u64)zone->name == 0xffffffff8180f072) { // DMA
            printf("(DMA)");
        } else if ((u64)zone->name == 0xffffffff817da3b0) {
            printf("(DMA32)");
        } else if ((u64)zone->name == 0xffffffff817da3b6) {
            printf("(Normal)");
        } else if ((u64)zone->name == 0xffffffff817da3bd) {
            printf("(Movable)");
        }
        printf(":\n");
        printf("    u64 watermark[3] = %#lx %#lx %#lx\n", zone->watermark[0],
                                                      zone->watermark[1],
                                                      zone->watermark[2]);
        printf("    u64 percpu_drift_mark = %#lx\n", zone->percpu_drift_mark);
        printf("    u64 lowmem_reserve[4] = %#lx %#lx %#lx %#lx\n", zone->lowmem_reserve[0],
                                                                zone->lowmem_reserve[1],
                                                                zone->lowmem_reserve[2],
                                                                zone->lowmem_reserve[3]);
        printf("    s32 node = %d\n", zone->node);
        printf("    u64 min_unmapped_pages = %#lx\n", zone->min_unmapped_pages);
        printf("    u64 min_slab_pages     = %#lx\n", zone->min_slab_pages);
        printf("    u64 pageset = %p\n", zone->pageset);
        printf("    u32 lock.slock = %#x\n", zone->lock.slock);
        printf("    s32 all_unreclaimable = %#x\n", zone->all_unreclaimable);
        printf("    u32 span_seqlock.sequence = %#x\n", zone->span_seqlock.sequence);
        printf("    u32 span_seqlock.lock.slock = %#x\n", zone->span_seqlock.lock.slock);
        for (j = 0; j < MAX_ORDER; j++) {
            free_area = &zone->free_area[j];
            printf("    free_area[%d]:\n", j);
            for (k = 0; k < MIGRATE_TYPES; k++) {
                page_lru_next = (uint64_t)free_area->free_list[k].next;
                page_lru_prev = (uint64_t)free_area->free_list[k].prev;
                page_lru_head = page_lru_next;
                printf("        u64 u64 free_list[%d]: next = %#lx, prev = %#lx\n", k, page_lru_next, page_lru_prev);
                if ((page_lru_next & PAGE_MASK) == PAGE_MASK && pages) {
                    pfn = (page_lru_head - 0x28 - PAGE_MASK) / 0x38;
                    printf("                               pfn: %x", pfn);
                    page = &pages[pfn];
                    if (page->private > 0) { // order
                        printf("-%x", pfn + (1 << page->private) - 1);
                    }
                    page_lru_current = (uint64_t)page->lru.next;
                    while ((page_lru_current & PAGE_MASK) == PAGE_MASK && page_lru_current != page_lru_head) {
                        pfn = (page_lru_current - 0x28 - PAGE_MASK) / 0x38;
                        printf(", %x", pfn);
                        page = &pages[pfn];
                        if (page->private > 0) { // order
                            printf("-%x", pfn + (1 << page->private) - 1);
                        }
                        page_lru_current = (uint64_t)page->lru.next;
                    }
                    printf("\n");
                }
            }
            printf("        u64 nr_free = %#lx\n", free_area->nr_free);
        }
        printf("    u32 compact_considered = %#x\n", zone->compact_considered);
        printf("    u32 compact_defer_shift = %#x\n", zone->compact_defer_shift);
        printf("    u32 lru_lock.slock = %#x\n", zone->lru_lock.slock);
        for (j = 0; j < NR_LRU_LISTS; j++) {
            printf("    u64 u64 lru[%d]: next = %p, prev = %p\n", j, zone->lru[j].list.next, zone->lru[j].list.prev);
        }
        printf("    ... skip reclaim_stat vm_stat ...\n");
        printf("    u64 wait_table = %p\n", zone->wait_table);
        printf("    u64 wait_table_hash_nr_entries = %#lx\n", zone->wait_table_hash_nr_entries);
        printf("    u64 wait_table_bits = %#lx\n", zone->wait_table_bits);
        printf("    u64 zone_pgdat = %p\n", zone->zone_pgdat);
        printf("    u64 zone_start_pfn  = %#lx\n", zone->zone_start_pfn);
        printf("    u64 spanned_pages   = %#lx\n", zone->spanned_pages);
        printf("    u64 present_pages   = %#lx\n", zone->present_pages);
        printf("    u64 name = %p\n", zone->name);
    }

    printf("\n");
    for (i = 0; i < MAX_ZONELISTS; i++) {
        printf("node_zonelists[%d]:\n", i);
        for (j = 0; j < MAX_ZONES_PER_ZONELIST + 1; j++) {
            printf("\t_zonerefs[%d]: zone_idx = %d, zone = %p\n",
                    j,
                    node_data->node_zonelists[i]._zonerefs[j].zone_idx,
                    node_data->node_zonelists[i]._zonerefs[j].zone
                );
        }
    }

    munmap(node_data, size);
    close(fd);

    if (pages) {
        munmap(pages, pagesSize);
        close(fd2);
    }

    return 0;
}
