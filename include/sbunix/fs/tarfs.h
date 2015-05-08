#ifndef _SBUNIX_TARFS_H
#define _SBUNIX_TARFS_H

#include <sbunix/fs/vfs.h>
/**
 * TARFS Functional Requirements:
 * open, read, close, opendir, readdir, closedir
 *
 * Some info about how TARFS is incorporated into SBUnix:
 * Kernel’s ELF headers tell loader what to load where (There will be a
 * section for code text, rodata, bss, …)
 *
 * We add an extra section containing our small files that gets loaded
 * along with the kernel (part of the kernel binary)
 *
 * Similar to initrd in Linux (in that, it provides a Filesystem before
 * there is a real Filesystem).
 *
 * Writing to files is not permitted
 *
 * Starts at ‘_binary_tarfs_start’ (Use its address (&) to find the start
 * of tarfs)
 *
 * If name doesn’t match, skip size bytes and continue.
 */

extern char _binary_tarfs_start; /* Starting label of tarfs in kernel memory */
extern char _binary_tarfs_end;   /* Ending label of tarfs in kernel memory */

extern struct file_ops tarfs_file_ops; /* File hooks implemented by Tarfs */

#define TARFS_NORMAL_FILE_1   '0'
#define TARFS_NORMAL_FILE_2   '\0'
#define TARFS_HARD_LINK       '1'
#define TARFS_SYMBOLIC_LINK   '2'
#define TARFS_CHARACTER       '3'
#define TARFS_BLOCK           '4'
#define TARFS_DIRECTORY       '5'

struct posix_header_ustar {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

uint64_t aotoi(char *optr, int length);
void test_aotoi(void);

static inline int tarfs_normal_type(struct posix_header_ustar *hd) {
    if(!hd)
        return 0;
    else
        return hd->typeflag == TARFS_NORMAL_FILE_1 ||
                hd->typeflag == TARFS_NORMAL_FILE_2;
}

/**
 * Return the first tarfs header in the file system
 */
static inline struct posix_header_ustar *tarfs_first(void) {
    if(&_binary_tarfs_end - &_binary_tarfs_start < 512){
        return NULL;
    } else {
        return (struct posix_header_ustar *)&_binary_tarfs_start;
    }
}

/**
 * Return the next tarfs header in the file system
 *
 * @cur_hdr: the current ustar header
 */
static inline struct posix_header_ustar *tarfs_next(struct posix_header_ustar *cur_hdr) {
    /* The last header will have a null name field */
    if(!cur_hdr || cur_hdr->name[0] == '\0'){
        return NULL;
    } else {
        uint64_t size = aotoi(cur_hdr->size, sizeof(cur_hdr->size));
        cur_hdr += 1 + size/512 + (size % 512 != 0);
        if(cur_hdr->name[0] == '\0')
            return NULL;
        else
            return cur_hdr;
    }
}

void test_read_tarfs(void);
void test_all_tarfs(const char *path);

/* Tarfs file operations */
struct file *tarfs_open(const char *path, int flags, mode_t mode, int *err);
off_t tarfs_lseek(struct file *fp, off_t offset, int origin);
ssize_t tarfs_read(struct file *fp, char *buf, size_t count, off_t *offset);
ssize_t tarfs_write(struct file *fp, const char *buf, size_t count,
                    off_t *offset);
//int tarfs_readdir(struct file *fp, void *dirent, filldir_t filldir);
int tarfs_close(struct file *fp);


long tarfs_isfile(const char *abspath);
long tarfs_isnormal(const char *abspath);
long tarfs_isdir(const char *rpath);

long tarfs_init(void);

#endif
