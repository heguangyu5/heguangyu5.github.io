#include <stdio.h>
#include <stdlib.h>
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

    return 0;
}
