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

#define ACPI_OEM_ID_SIZE    6

struct acpi_table_rsdp {
	char signature[8];	/* ACPI signature, contains "RSD PTR " */
	u8 checksum;		/* ACPI 1.0 checksum */
	char oem_id[ACPI_OEM_ID_SIZE];	/* OEM identification */
	u8 revision;		/* Must be (0) for ACPI 1.0 or (2) for ACPI 2.0+ */
	u32 rsdt_physical_address;	/* 32-bit physical address of the RSDT */
	u32 length;		/* Table length in bytes, including header (ACPI 2.0+) */
	u64 xsdt_physical_address;	/* 64-bit physical address of the XSDT (ACPI 2.0+) */
	u8 extended_checksum;	/* Checksum of entire table (ACPI 2.0+) */
	u8 reserved[3];		/* Reserved, must be zero */
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct acpi_table_rsdp);
    printf("sizeof(struct acpi_table_rsdp) = %d\n", size);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/acpi_table_rsdp.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct acpi_table_rsdp *rsdp;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    rsdp = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (rsdp == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("signature   = ");
    int i;
    for (i = 0; i < 8; i++) {
        printf("%c", rsdp->signature[i]);
    }
    printf("\n");
    printf("checksum    = %#x\n", rsdp->checksum);
    printf("oem_id      = %s\n", rsdp->oem_id);
    printf("revision    = %#x\n", rsdp->revision);
    printf("rsdt_physical_address   = %#x\n", rsdp->rsdt_physical_address);
    printf("length                  = %d\n", rsdp->length);
    printf("xsdt_physical_address   = %#lx\n", rsdp->xsdt_physical_address);
    printf("extended_checksum       = %#x\n", rsdp->extended_checksum);
    printf("reserved[3] = %d %d %d\n", rsdp->reserved[0], rsdp->reserved[1], rsdp->reserved[2]);

    munmap(rsdp, size);
    close(fd);

    return 0;
}
