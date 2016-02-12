#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

struct list_head {
    struct list_head *next, *prev;
};

typedef unsigned gfp_t;

typedef struct {
    int counter;
} atomic_t;

struct kref {
    atomic_t refcount;
};

struct kobject {
	const char		*name;
	struct list_head	entry;
	struct kobject		*parent;
	struct kset		*kset;
	struct kobj_type	*ktype;
	struct sysfs_dirent	*sd;
	struct kref		kref;
	unsigned int state_initialized:1;
	unsigned int state_in_sysfs:1;
	unsigned int state_add_uevent_sent:1;
	unsigned int state_remove_uevent_sent:1;
	unsigned int uevent_suppress:1;
};

struct kmem_cache_order_objects {
    unsigned long x;
};

struct kmem_cache {
	struct kmem_cache_cpu *cpu_slab;
	/* Used for retriving partial slabs etc */
	unsigned long flags;
	unsigned long min_partial;
	int size;		/* The size of an object including meta data */
	int objsize;		/* The size of an object without meta data */
	int offset;		/* Free pointer offset. */
	struct kmem_cache_order_objects oo;

	/* Allocation and freeing of slabs */
	struct kmem_cache_order_objects max;
	struct kmem_cache_order_objects min;
	gfp_t allocflags;	/* gfp flags to use on each alloc */
	int refcount;		/* Refcount for slab cache destroy */
	void (*ctor)(void *);
	int inuse;		/* Offset to metadata */
	int align;		/* Alignment */
	int reserved;		/* Reserved bytes at the end of slabs */
	const char *name;	/* Name (only for display!) */
	struct list_head list;	/* List of slab caches */

	struct kobject kobj;	/* For sysfs */

	/*
	 * Defragmentation by allocating from a remote node.
	 */
	int remote_node_defrag_ratio;
	struct kmem_cache_node *node;
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct kmem_cache);
    printf("sizeof(struct kmem_cache) = %#x\n", size);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/kmem_cache.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct kmem_cache *kmem_cache;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    kmem_cache = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (kmem_cache == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("cpu_slab = %p\n", kmem_cache->cpu_slab);
    printf("size = %#x, objsize = %#x, offset = %#x, inuse = %#x\n", kmem_cache->size,
                                                                     kmem_cache->objsize,
                                                                     kmem_cache->offset,
                                                                     kmem_cache->inuse);
    printf("oo = %#lx, order = %d, objects = %d\n", kmem_cache->oo.x,
                                                    (uint16_t)kmem_cache->oo.x >> 16,
                                                    (uint16_t)(kmem_cache->oo.x & 0xFFFF));
    printf("min = %#lx, order = %d, objects = %d\n", kmem_cache->min.x,
                                                     (uint16_t)kmem_cache->min.x >> 16,
                                                     (uint16_t)(kmem_cache->min.x & 0xFFFF));
    printf("align = %#x\n", kmem_cache->align);
    printf("reserved = %#x\n", kmem_cache->reserved);
    printf("list.prev = %p, list.next = %p\n", kmem_cache->list.prev, kmem_cache->list.next);
    printf("kmem_cache_node = %p\n", kmem_cache->node);

    munmap(kmem_cache, size);
    close(fd);

    return 0;
}
