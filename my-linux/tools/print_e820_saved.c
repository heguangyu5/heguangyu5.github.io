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

struct e820entry {
	__u64 addr;	/* start of memory segment */
	__u64 size;	/* size of memory segment */
	__u32 type;	/* type of memory segment */
} __attribute__((packed));

struct e820map {
	__u32 nr_map;
//#define E820_X_MAX 128 // 不太确定 E820_X_MAX 等于多少,因为不确定 __KERNEL__ 是否定义,先定为128吧
#define E820_X_MAX 320
	struct e820entry map[E820_X_MAX];
};

#define E820_RAM	1
#define E820_RESERVED	2
#define E820_ACPI	3
#define E820_NVS	4
#define E820_UNUSABLE	5

char *e820_type_list[] = {
    NULL,
    "E820_RAM",
    "E820_RESERVED",
    "E820_ACPI",
    "E820_NVS",
    "E820_UNUSABLE"
};

int main(int argc, char *argv[])
{
    printf("sizeof(struct e820_map) = %zd\n", sizeof(struct e820map));

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/e820_saved.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct stat buf;
    struct e820map *e820_saved;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (fstat(fd, &buf) == -1) {
        perror("fstat");
        return 1;
    }

    e820_saved = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (e820_saved == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("e820_saved.nr_map = %d\n", e820_saved->nr_map);

    int i;
    for (i = 0; i < e820_saved->nr_map; i++) {
        printf(
            "%d: %-16lx - %-16lx (%-16lx) %s\n",
            i,
            e820_saved->map[i].addr,
            e820_saved->map[i].addr + e820_saved->map[i].size,
            e820_saved->map[i].size,
            e820_type_list[e820_saved->map[i].type]
        );
    }


    munmap(e820_saved, buf.st_size);
    close(fd);

    return 0;
}
