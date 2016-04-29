#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s virt_addr\n", argv[0]);
        return 1;
    }

    uint64_t virt_addr = strtoul(argv[1], NULL, 0);
    if (errno == ERANGE) {
        perror("strtoul");
        printf("ULONG_MAX = %#lX\n", ULONG_MAX);
        return 1;
    }

    uint16_t PGDIR = (virt_addr & 0x0000FF8000000000) >> 39;
    uint16_t PUD   = (virt_addr & 0x0000007FC0000000) >> 30;
    uint16_t PMD   = (virt_addr & 0x000000003FE00000) >> 21;
    uint16_t PT    = (virt_addr & 0x00000000001FF000) >> 12;

    printf("%#lX => %#lX\n", virt_addr, virt_addr & 0x0000FFFFFFFFFFFF);
    printf("PGDIR = %#-5x = %3d = %#x\n", PGDIR, PGDIR, PGDIR * 8);
    printf("PUD   = %#-5x = %3d = %#x\n", PUD,   PUD,   PUD * 8);
    printf("PMD   = %#-5x = %3d = %#x\n", PMD,   PMD,   PMD * 8);
    printf("PT    = %#-5x = %3d = %#x\n", PT,    PT,    PT * 8);

    return 0;
}
