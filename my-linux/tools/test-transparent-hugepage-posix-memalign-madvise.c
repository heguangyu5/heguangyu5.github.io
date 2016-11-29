#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

int main(void)
{
    void *addr;
    int err = posix_memalign(&addr, 0x200000, 0x1000000);
    if (err != 0) {
        if (err == EINVAL) {
            printf("posix_memalign EINVAL\n");
        } else if (err == ENOMEM) {
            printf("posix_memalign ENOMEM\n");
        }
        return 1;
    }

    printf("posix_memalign ok, addr = %p\n", addr);

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

    return 0;
}
