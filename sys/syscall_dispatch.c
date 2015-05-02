#include "syscall_dispatch.h"
#include <sys/syscall.h>
#include <sbunix/asm.h>
#include <errno.h>
#include <sbunix/gdt.h>

/* 9th bit in the RFLAGS is the IF bit */
#define RFLAGS_IF   1<<9

/**
 * Enable syscalls on Intel/AMD x86_64 architecture.
 */
void enable_syscalls(void) {
    uint64_t efer;
    /* Enable the SYSCALL instr, set the EFER.SCE bit */
    efer = rdmsr(MSR_EFER);
    efer |= MSR_EFER_SCE;
    wrmsr(MSR_EFER, efer);

    /* _KERNEL_CS for syscall, _USER_CS for sysret */
    wrmsr(MSR_STAR, (uint64_t)_KERNEL_CS <<48 | (uint64_t)_USER_CS << 32);

    /* The function to dispatch system calls */
    wrmsr(MSR_LSTAR, (uint64_t)syscall_dispatch);

    /* We want interrupts off in the kernel */
    wrmsr(MSR_SFMASK, RFLAGS_IF);
}


int64_t syscall_dispatch(int64_t syscall_n) {
    switch(syscall_n) {
        case SYS_read:
        case SYS_write:
        case SYS_open:
        case SYS_close:
        case SYS_lseek:
        case SYS_mmap:
        case SYS_munmap:
        case SYS_brk:
        case SYS_pipe:
        case SYS_dup:
        case SYS_dup2:
        case SYS_nanosleep:
        case SYS_alarm:
        case SYS_getpid:
        case SYS_fork:
        case SYS_execve:
        case SYS_exit:
        case SYS_wait4:
        case SYS_uname:
        case SYS_getdents:
        case SYS_getcwd:
        case SYS_chdir:
        case SYS_getppid:
        default: return -ENOSYS;
    }
}
