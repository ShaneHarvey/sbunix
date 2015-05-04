#ifndef _SBUNIX_TERMINAL_H
#define _SBUNIX_TERMINAL_H

#include <sbunix/vfs/vfs.h>

#define EOT  4
#define EOF  -1

void term_putch(unsigned char c);

struct file *term_open(void);
off_t term_lseek(struct file *fp, off_t offset, int whence);
ssize_t term_read(struct file *fp, char *buf, size_t count, off_t *offset);
ssize_t term_write(struct file *fp, const char *buf, size_t count,
                   off_t *offset);
int term_close(struct file *fp);

#endif //_SBUNIX_TERMINAL_H