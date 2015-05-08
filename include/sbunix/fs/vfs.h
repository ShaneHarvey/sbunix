#ifndef _SBUNIX_VFS_VFS_H
#define _SBUNIX_VFS_VFS_H

#include <sys/types.h>
#include <sbunix/mm/types.h>

struct file {
    struct file_ops *f_op; /* file ops table */
    unsigned long f_count; /* file object's usage count */
    int f_flags;           /* flags specified on open */
    off_t f_pos;           /* file offset (file pointer) */
    unsigned long f_size;  /* file size */
    int f_error;           /* error code */
    void *private_data;    /* tty driver hook, for TARFS it points to file's ustar header */
};

/* whence for lseek */
enum { SEEK_SET = 0, SEEK_CUR = 1, SEEK_END = 2 };
/* flags for open */
enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };

struct file_ops {
    off_t (*lseek) (struct file *, off_t, int);
    ssize_t (*read) (struct file *, char *, size_t, off_t *);
    ssize_t (*write) (struct file *, const char *, size_t, off_t *);
//    int (*readdir) (struct file *, void *, filldir_t);
    int (*close) (struct file *);
};

char *resolve_path(const char *cwd, const char *path, long *err);

#endif //_SBUNIX_VFS_VFS_H