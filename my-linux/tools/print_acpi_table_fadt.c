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

struct acpi_generic_address {
	u8 space_id;		/* Address space where struct or register exists */
	u8 bit_width;		/* Size in bits of given register */
	u8 bit_offset;		/* Bit offset within the register */
	u8 access_width;	/* Minimum Access size (ACPI 3.0) */
	u64 address;		/* 64-bit address of struct or register */
};

struct acpi_table_fadt {
	struct acpi_table_header header;	/* Common ACPI table header */
	u32 facs;		/* 32-bit physical address of FACS */
	u32 dsdt;		/* 32-bit physical address of DSDT */
	u8 model;		/* System Interrupt Model (ACPI 1.0) - not used in ACPI 2.0+ */
	u8 preferred_profile;	/* Conveys preferred power management profile to OSPM. */
	u16 sci_interrupt;	/* System vector of SCI interrupt */
	u32 smi_command;	/* 32-bit Port address of SMI command port */
	u8 acpi_enable;		/* Value to write to smi_cmd to enable ACPI */
	u8 acpi_disable;	/* Value to write to smi_cmd to disable ACPI */
	u8 S4bios_request;	/* Value to write to SMI CMD to enter S4BIOS state */
	u8 pstate_control;	/* Processor performance state control */
	u32 pm1a_event_block;	/* 32-bit Port address of Power Mgt 1a Event Reg Blk */
	u32 pm1b_event_block;	/* 32-bit Port address of Power Mgt 1b Event Reg Blk */
	u32 pm1a_control_block;	/* 32-bit Port address of Power Mgt 1a Control Reg Blk */
	u32 pm1b_control_block;	/* 32-bit Port address of Power Mgt 1b Control Reg Blk */
	u32 pm2_control_block;	/* 32-bit Port address of Power Mgt 2 Control Reg Blk */
	u32 pm_timer_block;	/* 32-bit Port address of Power Mgt Timer Ctrl Reg Blk */
	u32 gpe0_block;		/* 32-bit Port address of General Purpose Event 0 Reg Blk */
	u32 gpe1_block;		/* 32-bit Port address of General Purpose Event 1 Reg Blk */
	u8 pm1_event_length;	/* Byte Length of ports at pm1x_event_block */
	u8 pm1_control_length;	/* Byte Length of ports at pm1x_control_block */
	u8 pm2_control_length;	/* Byte Length of ports at pm2_control_block */
	u8 pm_timer_length;	/* Byte Length of ports at pm_timer_block */
	u8 gpe0_block_length;	/* Byte Length of ports at gpe0_block */
	u8 gpe1_block_length;	/* Byte Length of ports at gpe1_block */
	u8 gpe1_base;		/* Offset in GPE number space where GPE1 events start */
	u8 cst_control;		/* Support for the _CST object and C States change notification */
	u16 C2latency;		/* Worst case HW latency to enter/exit C2 state */
	u16 C3latency;		/* Worst case HW latency to enter/exit C3 state */
	u16 flush_size;		/* Processor's memory cache line width, in bytes */
	u16 flush_stride;	/* Number of flush strides that need to be read */
	u8 duty_offset;		/* Processor duty cycle index in processor's P_CNT reg */
	u8 duty_width;		/* Processor duty cycle value bit width in P_CNT register */
	u8 day_alarm;		/* Index to day-of-month alarm in RTC CMOS RAM */
	u8 month_alarm;		/* Index to month-of-year alarm in RTC CMOS RAM */
	u8 century;		/* Index to century in RTC CMOS RAM */
	u16 boot_flags;		/* IA-PC Boot Architecture Flags (see below for individual flags) */
	u8 reserved;		/* Reserved, must be zero */
	u32 flags;		/* Miscellaneous flag bits (see below for individual flags) */
	struct acpi_generic_address reset_register;	/* 64-bit address of the Reset register */
	u8 reset_value;		/* Value to write to the reset_register port to reset the system */
	u8 reserved4[3];	/* Reserved, must be zero */
	u64 Xfacs;		/* 64-bit physical address of FACS */
	u64 Xdsdt;		/* 64-bit physical address of DSDT */
	struct acpi_generic_address xpm1a_event_block;	/* 64-bit Extended Power Mgt 1a Event Reg Blk address */
	struct acpi_generic_address xpm1b_event_block;	/* 64-bit Extended Power Mgt 1b Event Reg Blk address */
	struct acpi_generic_address xpm1a_control_block;	/* 64-bit Extended Power Mgt 1a Control Reg Blk address */
	struct acpi_generic_address xpm1b_control_block;	/* 64-bit Extended Power Mgt 1b Control Reg Blk address */
	struct acpi_generic_address xpm2_control_block;	/* 64-bit Extended Power Mgt 2 Control Reg Blk address */
	struct acpi_generic_address xpm_timer_block;	/* 64-bit Extended Power Mgt Timer Ctrl Reg Blk address */
	struct acpi_generic_address xgpe0_block;	/* 64-bit Extended General Purpose Event 0 Reg Blk address */
	struct acpi_generic_address xgpe1_block;	/* 64-bit Extended General Purpose Event 1 Reg Blk address */
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct acpi_table_fadt);
    printf("sizeof(struct acpi_table_fadt) = %d\n", size);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/acpi_table_fadt.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct acpi_table_fadt *fadt;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    fadt = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (fadt == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("header.signature    = %*.*s\n",    ACPI_NAME_SIZE, ACPI_NAME_SIZE, fadt->header.signature);
    printf("header.length       = %d\n",       fadt->header.length);
    printf("header.revision     = %d\n",       fadt->header.revision);
    printf("header.checksum     = %#x\n",      fadt->header.checksum);
    printf("header.oem_id       = %*.*s\n",    ACPI_OEM_ID_SIZE, ACPI_OEM_ID_SIZE, fadt->header.oem_id);
    printf("header.oem_table_id = %*.*s\n",    ACPI_OEM_TABLE_ID_SIZE, ACPI_OEM_TABLE_ID_SIZE, fadt->header.oem_table_id);
    printf("header.oem_revision = %d\n",       fadt->header.oem_revision);
    printf("header.asl_compiler_id   = %*.*s\n",   ACPI_NAME_SIZE, ACPI_NAME_SIZE, fadt->header.asl_compiler_id);
    printf("header.asl_compiler_revision = %d\n",  fadt->header.asl_compiler_revision);
    printf("facs = %#x\n", fadt->facs);
    printf("dsdt = %#x\n", fadt->dsdt);
    printf("model = %#x\n", fadt->model);
    printf("sci_interrupt = %#x\n", fadt->sci_interrupt);
    printf("pm_timer_block = %#x\n", fadt->pm_timer_block);

    munmap(fadt, size);
    close(fd);

    return 0;
}
