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

struct resource {
	u64 start;
	u64 end;
	const char *name;
	unsigned long flags;
	struct resource *parent, *sibling, *child;
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct resource);
    printf("sizeof(struct resource) = %d\n", size);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/resource.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct resource *resource;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    resource = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (resource == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("start = %#lx, end = %#lx, name = %p, flags = %#lx\n", resource->start, resource->end, resource->name, resource->flags);
    printf("parent = %p, sibling = %p, child = %p\n", resource->parent, resource->sibling, resource->child);

    munmap(resource, size);
    close(fd);

    return 0;
}
