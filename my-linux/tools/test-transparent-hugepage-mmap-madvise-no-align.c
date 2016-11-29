#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>

int main(void)
{
    void *addr = mmap(
        NULL,
        0x1000000,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    if (addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("mmap ok, addr = %p\n", addr);

    // 计算一下madvise的开始地址和长度
    void *addr_aligned = addr + (0x200000 - ((uint64_t)addr & 0xFFFFFF) % 0x200000);
    size_t length = 0x200000 * ((addr + 0x1000000 - addr_aligned) / 0x200000);
    printf("addr_aligned = %p, length = %#lx\n", addr_aligned, length);

    if (madvise(addr, 0x1000000, MADV_HUGEPAGE) == -1) {
        perror("madvise");
        return 1;
    }
    printf("madvise ok\n");

    char *s = (char *)addr;
    s[0] = 'H';
    s[1] = 'i';
    s[2] = 0;
    printf("memory content: %s\n", s);

    s = (char *)addr_aligned;
    s[0] = 'H';
    s[1] = 'i';
    s[2] = '2';
    s[3] = 0;
    printf("aligned content: %s\n", s);

    return 0;
}
