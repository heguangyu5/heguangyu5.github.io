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


struct list_head {
    struct list_head *next, *prev;
};

typedef struct spinlock {
	unsigned int slock;
} spinlock_t;

typedef struct {
    int counter;
} atomic_t;

enum pageflags {
	PG_locked,		/* Page is locked. Don't touch. */
	PG_error,
	PG_referenced,
	PG_uptodate,
	PG_dirty,
	PG_lru,
	PG_active,
	PG_slab,
	PG_owner_priv_1,	/* Owner use. If pagecache, fs may use*/
	PG_arch_1,
	PG_reserved,
	PG_private,		/* If pagecache, has fs-private data */
	PG_private_2,		/* If pagecache, has fs aux data */
	PG_writeback,		/* Page is under writeback */
	PG_head,		/* A head page */
	PG_tail,		/* A tail page */
	PG_swapcache,		/* Swap page: swp_entry_t in private */
	PG_mappedtodisk,	/* Has blocks allocated on-disk */
	PG_reclaim,		/* To be reclaimed asap */
	PG_swapbacked,		/* Page is backed by RAM/swap */
	PG_unevictable,		/* Page is "unevictable"  */
	PG_mlocked,		/* Page is vma mlocked */
	PG_uncached,		/* Page has been mapped as uncached */
	PG_hwpoison,		/* hardware poisoned page. Don't touch */
	PG_compound_lock,
	NR_PAGEFLAGS,

	/* Filesystems */
	PG_checked = PG_owner_priv_1,

	/* Two page bits are conscripted by FS-Cache to maintain local caching
	 * state.  These bits are set on pages belonging to the netfs's inodes
	 * when those inodes are being locally cached.
	 */
	PG_fscache = PG_private_2,	/* page backed by cache */

	/* XEN */
	PG_pinned = PG_owner_priv_1,
	PG_savepinned = PG_dirty,

	/* SLOB */
	PG_slob_free = PG_private,

	/* SLUB */
	PG_slub_frozen = PG_active,
};

char *pageflags_name[NR_PAGEFLAGS] = {
	"PG_locked",
	"PG_error",
	"PG_referenced",
	"PG_uptodate",
	"PG_dirty",
	"PG_lru",
	"PG_active",
	"PG_slab",
	"PG_owner_priv_1",
	"PG_arch_1",
	"PG_reserved",
	"PG_private",
	"PG_private_2",
	"PG_writeback",
	"PG_head",
	"PG_tail",
	"PG_swapcache",
	"PG_mappedtodisk",
	"PG_reclaim",
	"PG_swapbacked",
	"PG_unevictable",
	"PG_mlocked",
	"PG_uncached",
	"PG_hwpoison",
	"PG_compound_lock"
};

enum zone_type {
	ZONE_DMA,
	ZONE_DMA32,
	ZONE_NORMAL,
	ZONE_MOVABLE,
	MAX_NR_ZONES
};

char *zone_names[MAX_NR_ZONES] = {
	"ZONE_DMA",
	"ZONE_DMA32",
	"ZONE_NORMAL",
	"ZONE_MOVABLE"
};

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

static inline int variable_test_bit(int nr, const void *addr)
{
	u8 v;
	const u32 *p = (const u32 *)addr;

	asm("btl %2,%1; setc %0" : "=qm" (v) : "m" (*p), "Ir" (nr));
	return v;
}

int main(int argc, char *argv[])
{
    printf("SECTION_WIDTH = 19 (64TB/128M)\n");
    printf("NODES_WIDTH   = 6  (MAX_NUMNODES = 64)\n");
    printf("ZONE_WIDTH    = 2  (MAX_NR_ZONES = 4)\n");
    printf("NR_PAGEFLAGS  = %d\n\n", NR_PAGEFLAGS);

    printf("sizeof section_mem_map = 128M / 4K * sizeof(struct page) = 0x8000 * 0x38 = 0x1c0000 (1792K)\n");
    int size = 0x1c0000;

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/section_mem_map.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct page *pages;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    pages = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (pages == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    int i, j;
    u64 flags;
    for (i = 0; i < 0x8000; i++) {
#define NODE_MASK    0xFC00000000000000
#define ZONE_MASK    0x0300000000000000
        flags = pages[i].flags;
        if (flags == 0) {
            printf("PFN %#10x, flags = 0\n", i);
            continue;
        }
        printf("PFN %#10x, NODE %ld, %s,", i, (flags & NODE_MASK) >> (64-6), zone_names[(flags & ZONE_MASK) >> (64-6-2)]);
        printf(" flags ");
        for (j = 0; j < NR_PAGEFLAGS; j++) {
            if (variable_test_bit(j, &flags)) {
                printf("%s ", pageflags_name[j]);
            }
        }
        printf("\n");
    }

    munmap(pages, size);
    close(fd);

    return 0;
}
