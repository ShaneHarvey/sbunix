#ifndef SBUNIX_SYSCALL_DISPATCH_H
#define SBUNIX_SYSCALL_DISPATCH_H

#include <sys/defs.h>

extern uint64_t syscall_user_rsp;
extern uint64_t syscall_kernel_rsp;

void enable_syscalls(void);
int64_t syscall_dispatch(int64_t sysnum, int64_t a1, int64_t a2, int64_t a3,
                         int64_t a4, int64_t a5, int64_t a6);

#endif
