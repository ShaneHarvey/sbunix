#ifndef _SBUNIX_PIPE_H
#define _SBUNIX_PIPE_H

#include <sbunix/fs/vfs.h>

int pipe_open(struct file **read_end, struct file **write_end);

int pipe_can_mmap(struct file *fp);

off_t pipe_lseek(struct file *fp, off_t offset, int whence);
int pipe_readdir(struct file *filep, void *buf, unsigned int count);

/* Read end operations */
ssize_t read_end_read(struct file *fp, char *buf, size_t count,
                      off_t *offset);
ssize_t read_end_write(struct file *fp, const char *buf, size_t count,
                       off_t *offset);
int read_end_close(struct file *fp);

/* Write end operations */
ssize_t write_end_read(struct file *fp, char *buf, size_t count,
                      off_t *offset);
ssize_t write_end_write(struct file *fp, const char *buf, size_t count,
                       off_t *offset);
int write_end_close(struct file *fp);

#endif //_SBUNIX_PIPE_H
