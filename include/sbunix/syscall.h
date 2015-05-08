#ifndef _SBUNIX_SYSCALL_H
#define _SBUNIX_SYSCALL_H

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sbunix/time.h>
#include <dirent.h>
#include <errno.h>

struct mm_struct; /* forward declarations (from vmm.h) */

pid_t do_fork(void);

uint64_t do_brk(struct mm_struct *mm, uint64_t newbrk);

long do_execve(const char *filename, const char **argv, const char **envp, int rec);

pid_t do_wait4(pid_t pid, int *status, int options, struct rusage *rusage);

int do_kill(pid_t pid, int sig);

int do_nanosleep(const struct timespec *req, struct timespec *rem);

long do_getcwd(char *buf, size_t size);

long do_chdir(const char *path);

long do_open(const char *pathname, int flags, mode_t mode);

ssize_t do_read(int fd, void *buf, size_t count);

ssize_t do_write(int fd, const void *buf, size_t count);

off_t do_lseek(int fd, off_t offset, int whence);

int do_close(int fd);

int do_pipe(int *pipefd);

int do_dup(int oldfd);

int do_dup2(int oldfd, int newfd);

int do_uname(struct utsname *buf);

int do_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count);

void *do_mmap(void *addr, size_t length, int prot, int flags, int fd,
              off_t offset);

int do_munmap(void *addr, size_t length);

ssize_t do_getprocs(void *procbuf, size_t length);

#endif //_SBUNIX_SYSCALL_H
