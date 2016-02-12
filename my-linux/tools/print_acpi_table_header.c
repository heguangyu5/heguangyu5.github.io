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

int main(int argc, char *argv[])
{
    int size = sizeof(struct acpi_table_header);
    printf("sizeof(struct acpi_table_header) = %d\n", size);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/acpi_table_header.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct acpi_table_header *header;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    header = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (header == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("header.signature    = %*.*s\n",    ACPI_NAME_SIZE, ACPI_NAME_SIZE, header->signature);
    printf("header.length       = %d\n",       header->length);
    printf("header.revision     = %d\n",       header->revision);
    printf("header.checksum     = %#x\n",      header->checksum);
    printf("header.oem_id       = %*.*s\n",    ACPI_OEM_ID_SIZE, ACPI_OEM_ID_SIZE, header->oem_id);
    printf("header.oem_table_id = %*.*s\n",    ACPI_OEM_TABLE_ID_SIZE, ACPI_OEM_TABLE_ID_SIZE, header->oem_table_id);
    printf("header.oem_revision = %d\n",       header->oem_revision);
    printf("header.asl_compiler_id   = %*.*s\n",   ACPI_NAME_SIZE, ACPI_NAME_SIZE, header->asl_compiler_id);
    printf("header.asl_compiler_revision = %d\n",  header->asl_compiler_revision);

    munmap(header, size);
    close(fd);

    return 0;
}
