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
typedef int8_t s8;


#define  BINPRM_BUF_SIZE 128

struct linux_binprm {
	char buf[BINPRM_BUF_SIZE];
	struct vm_area_struct *vma;
	unsigned long vma_pages;
	struct mm_struct *mm;
	unsigned long p; /* current top of mem */
	unsigned int
		cred_prepared:1,/* true if creds already prepared (multiple
				 * preps happen for interpreters) */
		cap_effective:1;/* true if has elevated effective capabilities,
				 * false if not; except for init which inherits
				 * its parent's caps anyway */
	unsigned int recursion_depth;
	struct file * file;
	struct cred *cred;	/* new credentials */
	int unsafe;		/* how unsafe this exec is (mask of LSM_UNSAFE_*) */
	unsigned int per_clear;	/* bits to clear in current->personality */
	int argc, envc;
	const char * filename;	/* Name of binary as seen by procps */
	const char * interp;	/* Name of the binary really executed. Most
				   of the time same as filename, but could be
				   different for binfmt_{misc,script} */
	unsigned interp_flags;
	unsigned interp_data;
	unsigned long loader, exec;
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct linux_binprm);
    printf("sizeof(struct linux_binprm) = %#x\n", size);

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/bprm.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct linux_binprm *bprm;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    bprm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (bprm == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("argc = %d, envc = %d\n", bprm->argc, bprm->envc);
    printf("current top of mem = %#lx\n", bprm->p);
    printf("mm = %p\n", bprm->mm);
    printf("vma_pages = %#lx\n", bprm->vma_pages);
	printf("filename = %p\n", bprm->filename);
	printf("interp = %p\n", bprm->interp);

    munmap(bprm, size);
    close(fd);

    return 0;
}
