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

struct list_head {
    struct list_head *next, *prev;
};

typedef struct spinlock {
	unsigned int slock;
} spinlock_t;

struct __wait_queue_head {
    spinlock_t lock;
    struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

typedef struct {
    int counter;
} atomic_t;

struct mutex {
	/* 1: unlocked, 0: locked, negative: locked, possible waiters */
	atomic_t		count;
	spinlock_t		wait_lock;
	struct list_head	wait_list;
	struct thread_info	*owner;
};

typedef unsigned fmode_t;

struct rw_semaphore {
    long            count;
    spinlock_t      wait_lock;
    struct list_head    wait_list;
};

struct hlist_bl_head {
    struct hlist_bl_node *first;
};

typedef long long qsize_t;

struct mem_dqinfo {
	struct quota_format_type *dqi_format;
	int dqi_fmt_id;		/* Id of the dqi_format - used when turning
				 * quotas on after remount RW */
	struct list_head dqi_dirty_list;	/* List of dirty dquots */
	unsigned long dqi_flags;
	unsigned int dqi_bgrace;
	unsigned int dqi_igrace;
	qsize_t dqi_maxblimit;
	qsize_t dqi_maxilimit;
	void *dqi_priv;
};

#define MAXQUOTAS 2

struct quota_info {
	unsigned int flags;			/* Flags for diskquotas on this device */
	struct mutex dqio_mutex;		/* lock device while I/O in progress */
	struct mutex dqonoff_mutex;		/* Serialize quotaon & quotaoff */
	struct rw_semaphore dqptr_sem;		/* serialize ops using quota_info struct, pointers from inode to dquots */
	struct inode *files[MAXQUOTAS];		/* inodes of quotafiles */
	struct mem_dqinfo info[MAXQUOTAS];	/* Information for each quota type */
	const struct quota_format_ops *ops[MAXQUOTAS];	/* Operations for each type */
};

struct super_block {
	struct list_head	s_list;		/* Keep this first */
	u32			s_dev;		/* search index; _not_ kdev_t */
	unsigned char		s_dirt;
	unsigned char		s_blocksize_bits;
	unsigned long		s_blocksize;
	loff_t			s_maxbytes;	/* Max file size */
	struct file_system_type	*s_type;
	const struct super_operations	*s_op;
	const struct dquot_operations	*dq_op;
	const struct quotactl_ops	*s_qcop;
	const struct export_operations *s_export_op;
	unsigned long		s_flags;
	unsigned long		s_magic;
	struct dentry		*s_root;
	struct rw_semaphore	s_umount;
	struct mutex		s_lock;
	int			s_count;
	atomic_t		s_active;
	void                    *s_security;
	const struct xattr_handler **s_xattr;

	struct list_head	s_inodes;	/* all inodes */
	struct hlist_bl_head	s_anon;		/* anonymous dentries for (nfs) exporting */
	struct list_head  *s_files;
	/* s_dentry_lru, s_nr_dentry_unused protected by dcache.c lru locks */
	struct list_head	s_dentry_lru;	/* unused dentry lru */
	int			s_nr_dentry_unused;	/* # of dentry on lru */

	struct block_device	*s_bdev;
	struct backing_dev_info *s_bdi;
	struct mtd_info		*s_mtd;
	struct list_head	s_instances;
	struct quota_info	s_dquot;	/* Diskquota specific options */

	int			s_frozen;
	wait_queue_head_t	s_wait_unfrozen;

	char s_id[32];				/* Informational name */
	u8 s_uuid[16];				/* UUID */

	void 			*s_fs_info;	/* Filesystem private info */
	fmode_t			s_mode;

	/* Granularity of c/m/atime in ns.
	   Cannot be worse than a second */
	u32		   s_time_gran;

	/*
	 * The next field is for VFS *only*. No filesystems have any business
	 * even looking at it. You had been warned.
	 */
	struct mutex s_vfs_rename_mutex;	/* Kludge */

	/*
	 * Filesystem subtype.  If non-empty the filesystem type field
	 * in /proc/mounts will be "type.subtype"
	 */
	char *s_subtype;

	/*
	 * Saved mount options for lazy filesystems using
	 * generic_show_options()
	 */
	char *s_options;
	const struct dentry_operations *s_d_op; /* default d_op for dentries */
};

int main(int argc, char *argv[])
{
    int size = sizeof(struct super_block);
    printf("sizeof(struct super_block) = %#x\n", size);

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/super_block.memdump\n", argv[0]);
        return 1;
    }

    int fd;
    struct super_block *sb;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    sb = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (sb == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("s_id = %s\n", sb->s_id);
    printf("file_system_type = %p\n", sb->s_type);
    printf("super_operations = %p\n", sb->s_op);
    printf("s_fs_info = %p\n", sb->s_fs_info);
    printf("root dentry = %p\n", sb->s_root);
    printf("dentry operations = %p\n", sb->s_d_op);
	printf("s_op = %p\n", sb->s_op);

    munmap(sb, size);
    close(fd);

    return 0;
}
