#ifndef _SBUNIX_SYSCALL_H
#define _SBUNIX_SYSCALL_H

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sbunix/time.h>
#include <dirent.h>
#include <errno.h>

int do_execve(const char *filename, const char **argv, const char **envp);


int do_open(const char *pathname, int flags, mode_t mode);

ssize_t do_read(int fd, void *buf, size_t count);

ssize_t do_write(int fd, const void *buf, size_t count);

off_t do_lseek(int fd, off_t offset, int whence);

int do_close(int fd);

int do_pipe(int *pipefd);

int do_dup(int oldfd);

int do_dup2(int oldfd, int newfd);

int do_uname(struct utsname *buf);


#endif //_SBUNIX_SYSCALL_H
