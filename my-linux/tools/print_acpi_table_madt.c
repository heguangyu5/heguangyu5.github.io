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

#define ACPI_NAME_SIZE          4
#define ACPI_OEM_ID_SIZE        6
#define ACPI_OEM_TABLE_ID_SIZE  8

struct acpi_table_header {
	char signature[ACPI_NAME_SIZE];	/* ASCII table signature */
	u32 length;		/* Length of table in bytes, including this header */
	u8 revision;		/* ACPI Specification minor version # */
	u8 checksum;		/* To make sum of entire table == 0 */
	char oem_id[ACPI_OEM_ID_SIZE];	/* ASCII OEM identification */
	char oem_table_id[ACPI_OEM_TABLE_ID_SIZE];	/* ASCII OEM table identification */
	u32 oem_revision;	/* OEM revision number */
	char asl_compiler_id[ACPI_NAME_SIZE];	/* ASCII ASL compiler vendor ID */
	u32 asl_compiler_revision;	/* ASL compiler version */
};

struct acpi_table_madt {
	struct acpi_table_header header;	/* Common ACPI table header */
	u32 address;		/* Physical address of local APIC */
	u32 flags;
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct acpi_table_madt);
    printf("sizeof(struct acpi_table_madt) = %d\n", size);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/acpi_table_madt.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct acpi_table_madt *madt;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    madt = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (madt == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // header
    printf("madt.header.signature    = %*.*s\n",    ACPI_NAME_SIZE, ACPI_NAME_SIZE, madt->header.signature);
    printf("madt.header.length       = %d\n",       madt->header.length);
    printf("madt.header.revision     = %d\n",       madt->header.revision);
    printf("madt.header.checksum     = %#x\n",      madt->header.checksum);
    printf("madt.header.oem_id       = %*.*s\n",    ACPI_OEM_ID_SIZE, ACPI_OEM_ID_SIZE, madt->header.oem_id);
    printf("madt.header.oem_table_id = %*.*s\n",    ACPI_OEM_TABLE_ID_SIZE, ACPI_OEM_TABLE_ID_SIZE, madt->header.oem_table_id);
    printf("madt.header.oem_revision = %d\n",       madt->header.oem_revision);
    printf("madt.header.asl_compiler_id   = %*.*s\n",   ACPI_NAME_SIZE, ACPI_NAME_SIZE, madt->header.asl_compiler_id);
    printf("madt.header.asl_compiler_revision = %d\n",  madt->header.asl_compiler_revision);
    // address
    printf("madt.address = %#x\n", madt->address);
    // flags
    printf("madt.flags = %#x\n", madt->flags);

    munmap(madt, size);
    close(fd);

    return 0;
}
