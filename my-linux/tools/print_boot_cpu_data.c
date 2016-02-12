#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef uint8_t __u8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t __u32;
typedef uint64_t u64;

#define SMP_CACHE_BYTES 64
#define NCAPINTS    10

struct cpuinfo_x86 {
	__u8			x86;		/* CPU family */
	__u8			x86_vendor;	/* CPU vendor */
	__u8			x86_model;
	__u8			x86_mask;

	/* Number of 4K pages in DTLB/ITLB combined(in pages): */
	int			x86_tlbsize;

	__u8			x86_virt_bits;
	__u8			x86_phys_bits;
	/* CPUID returned core id bits: */
	__u8			x86_coreid_bits;
	/* Max extended CPUID function supported: */
	__u32			extended_cpuid_level;
	/* Maximum supported CPUID level, -1=no CPUID: */
	int			cpuid_level;
	__u32			x86_capability[NCAPINTS];
	char			x86_vendor_id[16];
	char			x86_model_id[64];
	/* in KB - valid for CPUS which support this call: */
	int			x86_cache_size;
	int			x86_cache_alignment;	/* In bytes */
	int			x86_power;
	unsigned long		loops_per_jiffy;
	/* cpuid returned max cores value: */
	u16			 x86_max_cores;
	u16			apicid;
	u16			initial_apicid;
	u16			x86_clflush_size;

	/* number of cores as seen by the OS: */
	u16			booted_cores;
	/* Physical processor id: */
	u16			phys_proc_id;
	/* Core id: */
	u16			cpu_core_id;
	/* Compute unit id */
	u8			compute_unit_id;
	/* Index into per_cpu list: */
	u16			cpu_index;
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

char *cpu_vendor_names[3] = {
    "Intel",
    "Cyrix",
    "AMD"
};


int main(int argc, char *argv[])
{
    int size = sizeof(struct cpuinfo_x86);
    printf("sizeof boot_cpu_data = sizeof(struct cpuinfo_x86) = %#x\n", size);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/boot_cpu_data.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct cpuinfo_x86 *boot_cpu_data;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    boot_cpu_data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (boot_cpu_data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("x86             = %#x\n", boot_cpu_data->x86);
    printf("x86_vendor      = %#x (%s)\n", boot_cpu_data->x86_vendor, cpu_vendor_names[boot_cpu_data->x86_vendor]);
    printf("x86_model       = %#x\n", boot_cpu_data->x86_model);
    printf("x86_mask        = %#x\n", boot_cpu_data->x86_mask);
    printf("x86_tlbsize     = %#x\n", boot_cpu_data->x86_tlbsize);
    printf("x86_virt_bits   = %#x\n", boot_cpu_data->x86_virt_bits);
    printf("x86_phys_bits   = %#x\n", boot_cpu_data->x86_phys_bits);
    printf("x86_coreid_bits = %#x\n", boot_cpu_data->x86_coreid_bits);
    printf("extended_cpuid_level = %#x\n", boot_cpu_data->extended_cpuid_level);
    printf("cpuid_level     = %#x\n", boot_cpu_data->cpuid_level);
    int i;
    for (i = 0; i < NCAPINTS; i++) {
        printf("x86_capability[%d] = %#x\n", i, boot_cpu_data->x86_capability[i]);
    }
    printf("x86_vendor_id       = %16.16s\n", boot_cpu_data->x86_vendor_id);
    printf("x86_model_id        = %64.64s\n", boot_cpu_data->x86_model_id);
    printf("x86_cache_size      = %#x\n", boot_cpu_data->x86_cache_size);
    printf("x86_cache_alignment = %#x\n", boot_cpu_data->x86_cache_alignment);
    printf("x86_power           = %#x\n", boot_cpu_data->x86_power);
    printf("loops_per_jiffy     = %#lx\n", boot_cpu_data->loops_per_jiffy);
    printf("x86_max_cores       = %#x\n", boot_cpu_data->x86_max_cores);
    printf("apicid              = %#x\n", boot_cpu_data->apicid);
    printf("initial_apicid      = %#x\n", boot_cpu_data->initial_apicid);
    printf("x86_clflush_size    = %#x\n", boot_cpu_data->x86_clflush_size);
    printf("booted_cores        = %#x\n", boot_cpu_data->booted_cores);
    printf("phys_proc_id        = %#x\n", boot_cpu_data->phys_proc_id);
    printf("cpu_core_id         = %#x\n", boot_cpu_data->cpu_core_id);
    printf("compute_unit_id     = %#x\n", boot_cpu_data->compute_unit_id);
    printf("cpu_index           = %#x\n", boot_cpu_data->cpu_index);

    munmap(boot_cpu_data, size);
    close(fd);

    return 0;
}
