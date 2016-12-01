#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

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

    char *s = (char *)addr;
    s[0] = 'H';
    s[1] = 'i';
    s[2] = 0;
    printf("memory content: %s\n", s);

    void *addr_aligned = addr + (0x200000 - ((uint64_t)addr & 0xFFFFFF) % 0x200000);
    printf("addr_aligned = %p\n", addr_aligned);

    s = (char *)addr_aligned;
    s[0] = 'H';
    s[1] = 'i';
    s[2] = '2';
    s[3] = 0;
    printf("aligned content: %s\n", s);

    return 0;
}
