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

typedef u64 acpi_physical_address;
typedef u8 acpi_owner_id;

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

union acpi_name_union {
	u32 integer;
	char ascii[4];
};

struct acpi_table_desc {
	acpi_physical_address address;
	struct acpi_table_header *pointer;
	u32 length;		/* Length fixed at 32 bits */
	union acpi_name_union signature;
	acpi_owner_id owner_id;
	u8 flags;
};

struct acpi_table_list {
	struct acpi_table_desc *tables;	/* Table descriptor array */
	u32 current_table_count;	/* Tables currently in the array */
	u32 max_table_count;	/* Max tables array will hold */
	u8 flags;
};

int main(int argc, char *argv[])
{
    int size_root_table_list = sizeof(struct acpi_table_list);
    int size_tables          = sizeof(struct acpi_table_desc) * 128;
    printf("sizeof(struct acpi_table_list acpi_gbl_root_table_list) = %d\n", size_root_table_list);
    printf("sizeof(struct acpi_table_desc initial_tables[128]) = %d\n", size_tables);

    if (argc != 3) {
        fprintf(stderr, "usage: %s /path/to/root_table_list.memdump /path/to/initial_tables.memdump\n", argv[0]);
        return 1;
    }

    int fd_list, fd_tables;
    struct acpi_table_list *root_table_list;
    struct acpi_table_desc *tables;

    fd_list = open(argv[1], O_RDONLY);
    if (fd_list == -1) {
        perror("open");
        return 1;
    }

    fd_tables = open(argv[2], O_RDONLY);
    if (fd_tables == -1) {
        perror("open");
        return 1;
    }

    root_table_list = mmap(NULL, size_root_table_list, PROT_READ, MAP_SHARED, fd_list, 0);
    if (root_table_list == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    tables = mmap(NULL, size_tables, PROT_READ, MAP_SHARED, fd_tables, 0);
    if (tables == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("root_table_list.current_table_count = %d\n", root_table_list->current_table_count);
    printf("root_table_list.max_table_count = %d\n", root_table_list->max_table_count);
    printf("root_table_list.flags = %#x\n", root_table_list->flags);
    printf("tables:\n");
    int i = 0;
    for (i = 0; i < root_table_list->current_table_count; i++) {
        printf("%2d: address = %#lx\n", i, tables[i].address);
        printf("    pointer = %p\n", tables[i].pointer);
        printf("    length  = %d\n", tables[i].length);
        printf("    signature = %4.4s\n", tables[i].signature.ascii);
        printf("    owner_id = %d\n", tables[i].owner_id);
        printf("    flags    = %#x\n", tables[i].flags);
    }


    munmap(root_table_list, size_root_table_list);
    munmap(tables, size_tables);
    close(fd_list);
    close(fd_tables);

    return 0;
}
