#ifndef _SBUNIX_TARFS_H
#define _SBUNIX_TARFS_H

#include <sbunix/sbunix.h>
#include <sbunix/vfs/vfs.h>

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

uint64_t aotoi(char *optr);
void test_aotoi(void);
void test_read_tarfs(void);

/* Tarfs file operations */
off_t tarfs_lseek(struct file *fp, off_t offset, int origin);
ssize_t tarfs_read(struct file *fp, char *buf, size_t count, off_t *offset);
ssize_t tarfs_write(struct file *fp, const char *buf, size_t count,
                    off_t *offset);
//int tarfs_readdir(struct file *fp, void *dirent, filldir_t filldir);
int tarfs_mmap(struct file *fp, struct vm_area *vma);
int tarfs_open(const char *path, struct file *fp);
unsigned long tarfs_get_unmapped_area(struct file *fp, unsigned long addr,
                                unsigned long len, unsigned long offset,
                                unsigned long flags);
int tarfs_check_flags(int flags);

#endif
