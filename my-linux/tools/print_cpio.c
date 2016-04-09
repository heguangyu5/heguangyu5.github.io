#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

// 根目录为什么nlink是6?
// 自身是一个,根目录里的..是一个, bin,dev,usr,sbin里的..各有1个
// 照此推论: bin,dev,sbin的nlink应该是2,usr的应该是4

struct cpio_newc_header {
   char    c_magic[6];
   char    c_ino[8];
   char    c_mode[8];
   char    c_uid[8];
   char    c_gid[8];
   char    c_nlink[8];
   char    c_mtime[8];
   char    c_filesize[8];
   char    c_devmajor[8];
   char    c_devminor[8];
   char    c_rdevmajor[8];
   char    c_rdevminor[8];
   char    c_namesize[8];
   char    c_check[8];
};

struct cpio_header {
    char magic[7];
    uint32_t ino;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t nlink;
    uint32_t mtime;
    uint32_t filesize;
    uint32_t devmajor;
    uint32_t devminor;
    uint32_t rdevmajor;
    uint32_t rdevminor;
    uint32_t namesize;
    uint32_t check;
};

uint32_t hex2int(char *p) {
    char buf[9];
    buf[8] = 0;
    memcpy(buf, p, 8);
    return strtol(buf, NULL, 16);
}

struct cpio_header h;
void new_header(char *p) {
    memset(&h, 0, sizeof(struct cpio_header));

    memcpy(&h.magic, p, 6);
    p += 6;

    h.ino  = hex2int(p); p += 8;
    h.mode = hex2int(p); p += 8;
    h.uid  = hex2int(p); p += 8;
    h.gid  = hex2int(p); p += 8;
    h.nlink = hex2int(p); p += 8;
    h.mtime = hex2int(p); p += 8;
    h.filesize = hex2int(p); p += 8;
    h.devmajor = hex2int(p); p += 8;
    h.devminor = hex2int(p); p += 8;
    h.rdevmajor = hex2int(p); p += 8;
    h.rdevminor = hex2int(p); p += 8;
    h.namesize = hex2int(p); p += 8;
}

int main(int argc, char *argv[])
{
    int headerLen = sizeof(struct cpio_newc_header);
    printf("sizeof(struct cpio_newc_header) = %d\n", headerLen);

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/cpio.archive\n", argv[0]);
        return 1;
    }

    int fd;
    struct stat buf;
    char *p;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (fstat(fd, &buf) == -1) {
        perror("fstat");
        return 1;
    }

    p = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    int pos = 0;
    char *ptr = p;
    char *name;
    char *file;
    int i;
    while (1) {
        // header
        new_header(ptr);
        ptr += headerLen;
        pos += headerLen;
        // name
        name = ptr;
        if (strcmp(name, "TRAILER!!!") == 0) {
            printf("\nTRAILER!!!\n");
            break;
        }
        ptr += h.namesize;
        pos += h.namesize;
        if (pos % 4 > 0) {
            ptr += 4 - (pos % 4);
            pos += 4 - (pos % 4);
        }
        // file
        if (h.filesize > 0) {
            file = ptr;
            ptr += h.filesize;
            pos += h.filesize;
            if (pos % 4 > 0) {
                ptr += 4 - (pos % 4);
                pos += 4 - (pos % 4);
            }
        }
        // print
        printf("%s   => ", name);
        if (h.filesize == 0) {
            printf("Empty");
        } else {
            for (i = 0; i < (h.filesize > 30 ? 30 : h.filesize); i++) {
                printf("%c", file[i]);
            }
        }
        printf("\tmagic: %s, ino: %d, mode: %o, uid: %d, gid: %d, nlink: %d, mtime = %d, filesize = %d, devmajor: %d, devminor = %d, rdevmajor = %d, rdevminor = %d, namesize = %d\n",
                h.magic, h.ino, h.mode, h.uid, h.gid, h.nlink, h.mtime, h.filesize,
                h.devmajor, h.devminor, h.rdevmajor, h.rdevminor, h.namesize);
    }



    munmap(p, buf.st_size);
    close(fd);

    return 0;
}
