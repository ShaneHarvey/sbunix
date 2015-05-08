#ifndef _SBUNIX_TERMINAL_H
#define _SBUNIX_TERMINAL_H

#include <sbunix/fs/vfs.h>

#define ETX  3
#define CTRL_C ETX
#define EOT  4
#define EOF  -1

int curr_tab_to_spaces(void);

void term_putch(unsigned char c);

struct file *term_open(void);
off_t term_lseek(struct file *fp, off_t offset, int whence);
int term_readdir(struct file *filep, void *buf, unsigned int count);
ssize_t term_read(struct file *fp, char *buf, size_t count, off_t *offset);
ssize_t term_write(struct file *fp, const char *buf, size_t count,
                   off_t *offset);
int term_close(struct file *fp);

int term_can_mmap(struct file *fp);

void test_terminal(void);

#endif //_SBUNIX_TERMINAL_H
