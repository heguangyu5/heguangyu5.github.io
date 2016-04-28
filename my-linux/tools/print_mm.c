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


struct rb_node
{
	unsigned long  rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));
    /* The alignment might seem pointless, but allegedly CRIS needs it */

struct rb_root
{
	struct rb_node *rb_node;
};

typedef struct {
    int counter;
} atomic_t;

typedef struct spinlock {
	unsigned int slock;
} spinlock_t;

struct list_head {
    struct list_head *next, *prev;
};

struct rw_semaphore {
    long            count;
    spinlock_t      wait_lock;
    struct list_head    wait_list;
};

enum {
	MM_FILEPAGES,
	MM_ANONPAGES,
	MM_SWAPENTS,
	NR_MM_COUNTERS
};


struct mutex {
	/* 1: unlocked, 0: locked, negative: locked, possible waiters */
	atomic_t		count;
	spinlock_t		wait_lock;
	struct list_head	wait_list;
	struct thread_info	*owner;
};

typedef struct {
	void *ldt;
	int size;
	struct mutex lock;
	void *vdso;
	unsigned short ia32_compat;
} mm_context_t;

struct hlist_head {
    struct hlist_node *first;
};

typedef struct page *pgtable_t;

struct mm_struct {
	struct vm_area_struct * mmap;		/* list of VMAs */
	struct rb_root mm_rb;
	struct vm_area_struct * mmap_cache;	/* last find_vma result */
	unsigned long (*get_unmapped_area) (struct file *filp,
				unsigned long addr, unsigned long len,
				unsigned long pgoff, unsigned long flags);
	void (*unmap_area) (struct mm_struct *mm, unsigned long addr);
	unsigned long mmap_base;		/* base of mmap area */
	unsigned long task_size;		/* size of task vm space */
	unsigned long cached_hole_size; 	/* if non-zero, the largest hole below free_area_cache */
	unsigned long free_area_cache;		/* first hole of size cached_hole_size or larger */
	unsigned long *pgd;
	atomic_t mm_users;			/* How many users with user space? */
	atomic_t mm_count;			/* How many references to "struct mm_struct" (users count as 1) */
	int map_count;				/* number of VMAs */

	spinlock_t page_table_lock;		/* Protects page tables and some counters */
	struct rw_semaphore mmap_sem;

	struct list_head mmlist;		/* List of maybe swapped mm's.	These are globally strung
						 * together off init_mm.mmlist, and are protected
						 * by mmlist_lock
						 */


	unsigned long hiwater_rss;	/* High-watermark of RSS usage */
	unsigned long hiwater_vm;	/* High-water virtual memory usage */

	unsigned long total_vm, locked_vm, shared_vm, exec_vm;
	unsigned long stack_vm, reserved_vm, def_flags, nr_ptes;
	unsigned long start_code, end_code, start_data, end_data;
	unsigned long start_brk, brk, start_stack;
	unsigned long arg_start, arg_end, env_start, env_end;
#define AT_VECTOR_SIZE 40
	unsigned long saved_auxv[AT_VECTOR_SIZE]; /* for /proc/PID/auxv */

	/*
	 * Special counters, in some configurations protected by the
	 * page_table_lock, in other configurations by being atomic.
	 */
	u64 rss_stat[NR_MM_COUNTERS];

	struct linux_binfmt *binfmt;

	// cpumask_t cpu_vm_mask;
	u8 cpu_vm_mask[32];

	/* Architecture-specific MM context */
	mm_context_t context;

	/* Swap token stuff */
	/*
	 * Last value of global fault stamp as seen by this process.
	 * In other words, this value gives an indication of how long
	 * it has been since this task got the token.
	 * Look at mm/thrash.c
	 */
	unsigned int faultstamp;
	unsigned int token_priority;
	unsigned int last_interval;

	/* How many tasks sharing this mm are OOM_DISABLE */
	atomic_t oom_disable_count;

	unsigned long flags; /* Must use atomic bitops to access the bits */

	struct core_state *core_state; /* coredumping support */
	spinlock_t		ioctx_lock;
	struct hlist_head	ioctx_list;
	/*
	 * "owner" points to a task that is regarded as the canonical
	 * user/owner of this mm. All of the following must be true in
	 * order for it to be changed:
	 *
	 * current == mm->owner
	 * current->mm != mm
	 * new_owner->mm == mm
	 * new_owner->alloc_lock is held
	 */
	struct task_struct *owner;

	/* store ref to file /proc/<pid>/exe symlink points to */
	struct file *exe_file;
	unsigned long num_exe_file_vmas;
	struct mmu_notifier_mm *mmu_notifier_mm;
	pgtable_t pmd_huge_pte; /* protected by page_table_lock */
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct mm_struct);
    printf("sizeof(struct mm_struct) = %#x\n", size);

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/mm.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct mm_struct *mm;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    mm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mm == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("pgd         = %p\n", mm->pgd);
    printf("task_size   = %#lx\n", mm->task_size);
    printf("hiwater_rss = %#lx\n", mm->hiwater_rss);
    printf("hiwater_vm  = %#lx\n", mm->hiwater_vm);
    printf("total_vm    = %#lx\n", mm->total_vm);
    printf("locked_vm   = %#lx\n", mm->locked_vm);
    printf("shared_vm   = %#lx\n", mm->shared_vm);
    printf("exec_vm     = %#lx\n", mm->exec_vm);
    printf("stack_vm    = %#lx\n", mm->stack_vm);
    printf("reserved_vm = %#lx\n", mm->reserved_vm);
    printf("nr_ptes     = %#lx\n", mm->nr_ptes);
    printf("start_code  = %#lx\n", mm->start_code);
    printf("end_code    = %#lx\n", mm->end_code);
    printf("start_data  = %#lx\n", mm->start_data);
    printf("start_brk   = %#lx\n", mm->start_brk);
    printf("brk         = %#lx\n", mm->brk);
    printf("start_stack = %#lx\n", mm->start_stack);
    printf("arg_start   = %#lx\n", mm->arg_start);
    printf("arg_end     = %#lx\n", mm->arg_end);
    printf("env_start   = %#lx\n", mm->env_start);
    printf("env_end     = %#lx\n", mm->env_end);
    printf("mmap_base   = %#lx\n", mm->mmap_base);
    printf("mmap        = %p\n", mm->mmap);

    printf("rss_stat[MM_FILEPAGES] = %#lx\n", mm->rss_stat[0]);
    printf("rss_stat[MM_ANONPAGES] = %#lx\n", mm->rss_stat[1]);
    printf("rss_stat[MM_SWAPENTS]  = %#lx\n", mm->rss_stat[2]);

    printf("saved_auxv:\n");
    int i;
    for (i = 0; i < AT_VECTOR_SIZE; i++) {
        printf("    %d: %#lx\n", i, mm->saved_auxv[i]);
    }

    munmap(mm, size);
    close(fd);

    return 0;
}
