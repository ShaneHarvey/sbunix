#include "syscall_dispatch.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <sbunix/time.h>
#include <sbunix/asm.h>
#include <errno.h>
#include <sbunix/gdt.h>
#include <sbunix/sbunix.h>

/* 9th bit in the RFLAGS is the IF bit */
#define RFLAGS_IF   1<<9

/* From syscall_entry.s */
extern void syscall_entry(void);

uint64_t syscall_user_rsp = 0;
uint64_t syscall_kernel_rsp = 0;

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
    //debug("Write STAR <-- 0x%lx\n", (uint64_t)_KERNEL_CS << 32 | (uint64_t)_USER_CS << 48);
    wrmsr(MSR_STAR, (uint64_t)_KERNEL_CS << 32 | (uint64_t)_USER_CS << 48);

    /* The function to enter on system calls */
    //debug("Write LSTAR <-- %lx\n", (uint64_t)syscall_entry);
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);

    /* We want interrupts off in the kernel */
    wrmsr(MSR_SFMASK, RFLAGS_IF);
}


int64_t syscall_dispatch(int64_t a1, int64_t a2, int64_t a3,
                         int64_t a4, int64_t a5, int64_t a6, int64_t sysnum) {
    int64_t err;
    debug("Doing a syscall: %d\n", sysnum);
    switch(sysnum) {
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
        default: err = -ENOSYS;
    }
    debug("Did a syscall: %d\n", sysnum);
    return err;
}


void sys_exit(int status) {
    return;
}

int sys_brk(void *addr) {
    return -ENOSYS;
}

pid_t sys_fork(void) {
    return -ENOSYS;
}


pid_t sys_getpid(void) {
    return -ENOSYS;
}

pid_t sys_getppid(void) {
    return -ENOSYS;
}

int sys_execve(const char *filename, char *const argv[], char *const envp[]) {
    return -ENOSYS;
}

pid_t sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage) {
    return -ENOSYS;
}

int sys_nanosleep(const struct timespec *req, struct timespec *rem) {
    return -ENOSYS;
}

unsigned int sys_alarm(unsigned int seconds)  {
    return -ENOSYS;
}

char *sys_getcwd(char *buf, size_t size) {
    return (void*)-ENOSYS;
}

int sys_chdir(const char *path) {
    return -ENOSYS;
}

int sys_open(const char *pathname, int flags) {
    return -ENOSYS;
}

ssize_t sys_read(int fd, void *buf, size_t count) {
    return -ENOSYS;
}

ssize_t sys_write(int fd, const void *buf, size_t count) {
    return -ENOSYS;
}

off_t sys_lseek(int fildes, off_t offset, int whence) {
    return -ENOSYS;
}

int sys_close(int fd) {
    return -ENOSYS;
}

int sys_pipe(int filedes[2]) {
    return -ENOSYS;
}

int sys_dup(int oldfd) {
    return -ENOSYS;
}

int sys_dup2(int oldfd, int newfd) {
    return -ENOSYS;
}

int sys_getdents(unsigned int fd, struct dirent *dirp, unsigned int count) {
    return -ENOSYS;
}

int sys_uname(struct utsname *buf) {
    return -ENOSYS;
}

void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
    return (void*)-ENOSYS;
}

int sys_munmap(void *addr, size_t length) {
    return -ENOSYS;
}
