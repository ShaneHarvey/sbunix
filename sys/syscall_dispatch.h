#ifndef SBUNIX_SYSCALL_DISPATCH_H
#define SBUNIX_SYSCALL_DISPATCH_H

#include <sys/defs.h>

void enable_syscalls(void);
int64_t syscall_dispatch(int64_t syscall_n);

#endif
