#ifndef _SBUNIX_TERMINAL_H
#define _SBUNIX_TERMINAL_H

#include <sbunix/vfs/vfs.h>

#define EOT  4
#define EOF  -1

void term_putch(unsigned char c);

ssize_t term_read(struct file *fp, char *buf, size_t count, off_t *offset);

#endif //_SBUNIX_TERMINAL_H
