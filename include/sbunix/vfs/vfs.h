#ifndef _SBUNIX_VFS_VFS_H
#define _SBUNIX_VFS_VFS_H

#include <sys/types.h>
#include <sbunix/mm/types.h>

struct file_ops {
    off_t (*lseek) (struct file *, off_t, int);
    ssize_t (*read) (struct file *, char *, size_t, off_t *);
    ssize_t (*write) (struct file *, const char *, size_t, off_t *);
//    int (*readdir) (struct file *, void *, filldir_t);
    int (*mmap) (struct file *, struct vm_area *);
    int (*open) (const char *, struct file *);
    unsigned long (*get_unmapped_area) (struct file *, unsigned long,
                                        unsigned long, unsigned long,
                                        unsigned long);
    int (*check_flags) (int flags);
};

struct file {
    struct file_ops *f_op; /* file ops table */
    unsigned long f_count; /* file object's usage count */
    unsigned int f_flags;  /* flags specified on open */
    off_t f_pos;           /* file offset (file pointer) */
    unsigned long f_size;  /* file size */
    int f_error;           /* error code */
    void *private_data;    /* tty driver hook, for TARFS it points to file's ustar header */
};

#endif //_SBUNIX_VFS_VFS_H