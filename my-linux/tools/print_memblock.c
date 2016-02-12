#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef uint64_t phys_addr_t;

#define INIT_MEMBLOCK_REGIONS	128

struct memblock_region {
	phys_addr_t base;
	phys_addr_t size;
};

struct memblock_type {
	unsigned long cnt;	/* number of regions */
	unsigned long max;	/* size of the allocated array */
	struct memblock_region *regions;
};

struct memblock {
	phys_addr_t current_limit;
	phys_addr_t memory_size;	/* Updated by memblock_analyze() */
	struct memblock_type memory;
	struct memblock_type reserved;
};

int main(int argc, char *argv[])
{
    printf("sizeof(struct memblock) = %zd\n", sizeof(struct memblock));
    printf("sizeof memblock.memory/reserved = INIT_MEMBLOCK_REGIONS * sizeof(struct memblock_region) = %zd\n", INIT_MEMBLOCK_REGIONS * sizeof(struct memblock_region));

    if (argc != 4) {
        fprintf(stderr, "usage: %s /path/to/memblock.memdump /path/to/memblock_memory.memdump /path/to/memblock_reserved.memdump\n", argv[0]);
        return 1;
    }

    int fdm, fdmm, fdmr;
    struct stat bufm, bufmm, bufmr;
    struct memblock *m;
    struct memblock_region *mm, *mr;
    int i;

    fdm = open(argv[1], O_RDONLY);
    if (fdm == -1) {
        perror("open memblock.memdump");
        return 1;
    }

    fdmm = open(argv[2], O_RDONLY);
    if (fdmm == -1) {
        perror("open memblock.memory.memdump");
        return 1;
    }

    fdmr = open(argv[3], O_RDONLY);
    if (fdmr == -1) {
        perror("open memblock.reserved.memdump");
        return 1;
    }

    if (fstat(fdm, &bufm) == -1) {
        perror("fstat memblock.memdump");
        return 1;
    }

    if (fstat(fdmm, &bufmm) == -1) {
        perror("fstat memblock.memory.memdump");
        return 1;
    }

    if (fstat(fdmr, &bufmr) == -1) {
        perror("stat memblock.reserved.memdump");
        return 1;
    }

    m = mmap(NULL, bufm.st_size, PROT_READ, MAP_SHARED, fdm, 0);
    if (m == MAP_FAILED) {
        perror("mmap memblock.memdump");
        return 1;
    }

    mm = mmap(NULL, bufmm.st_size, PROT_READ, MAP_SHARED, fdmm, 0);
    if (mm == MAP_FAILED) {
        perror("mmap memblock.memory.memdump");
        return 1;
    }

    mr = mmap(NULL, bufmr.st_size, PROT_READ, MAP_SHARED, fdmr, 0);
    if (mr == MAP_FAILED) {
        perror("mmap memblock.reserved.memdump");
        return 1;
    }

    printf("memblock.current_limit  = %#lx\n", m->current_limit);
    printf("memblock.memory_size    = %#lx\n", m->memory_size);
    printf("memblock.memory.cnt     = %#lx\n", m->memory.cnt);
    printf("memblock.memory.max     = %#lx\n", m->memory.max);
    printf("memblock.memory.regions = %p\n",  m->memory.regions);
    printf("memblock.reserved.cnt     = %#lx\n", m->reserved.cnt);
    printf("memblock.reserved.max     = %#lx\n", m->reserved.max);
    printf("memblock.reserved.regions = %p\n",  m->reserved.regions);

    printf("--memory regions--\n");
    for (i = 0; i < m->memory.cnt; i++) {
        printf("%d: start=%#lx, end=%#lx, size=%#lx\n", i, mm[i].base, mm[i].base + mm[i].size, mm[i].size);
    }

    printf("--reserved regions--\n");
    for (i = 0; i < m->reserved.cnt; i++) {
        printf("%d: start=%#lx, end=%#lx, size=%#lx\n", i, mr[i].base, mr[i].base + mr[i].size, mr[i].size);
    }

    munmap(m, bufm.st_size);
    munmap(mm, bufmm.st_size);
    munmap(mr, bufmr.st_size);
    close(fdm);
    close(fdmm);
    close(fdmr);

    return 0;
}
