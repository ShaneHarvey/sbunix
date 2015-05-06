#include "syscall_dispatch.h"
#include <sbunix/syscall.h>
#include <sbunix/asm.h>
#include <sbunix/gdt.h>
#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/mm/vmm.h>

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

void sys_exit(int status) {
    kill_curr_task(status);
    return;
}

uint64_t sys_brk(void *addr) {
    return do_brk(curr_task->mm, (uint64_t)addr);
}

pid_t sys_fork(void) {
    struct task_struct *child;
    uint64_t stack_off, curr_stack, child_stack;
    pid_t child_pid;

    child = fork_curr_task();
    if(!child)
        return (pid_t)-ENOMEM;
    /* Magic hack to switch to child task */
    child_pid = child->pid;
    child->first_switch = 1;
    /* offset of the child kstack the same as the current kstack */
    child_stack = ALIGN_UP(child->kernel_rsp, PAGE_SIZE);
    curr_stack = read_rsp();
    stack_off = ALIGN_UP(curr_stack, PAGE_SIZE) - curr_stack;
    child->kernel_rsp = child_stack - stack_off - 8;
    *(uint64_t *)child->kernel_rsp = (uint64_t)&&child_fork_ret;
    printk("curr_task RSP: %p, child_task RSP: %p\n",curr_stack, child->kernel_rsp);
    schedule();
    /* this will never happen, but the label gets optimized out if we don't trick gcc */
    if(child_pid == 0) {
child_fork_ret: /* Child will retq to this label the first time it gets scheduled */
        __asm__ __volatile__("xorq %rax, %rax; popq %rbx; retq;");
        return 0;
    }
    return child_pid;
}

pid_t sys_getpid(void) {
    return curr_task->pid;
}

pid_t sys_getppid(void) {
    if(!curr_task->parent)
        return 1;
    else
        return curr_task->parent->pid;
}

int sys_execve(const char *filename, const char **argv, const char **envp) {
    /* TODO: validate unbounded pointers ???? */
    return do_execve(filename, argv, envp);
}

pid_t sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage) {
    int err;

    /* status and rusage are filled in only if they are not NULL */
    if(status) {
        err = valid_userptr_write(curr_task->mm, status, sizeof(int));
        if(err)
            return (pid_t)err;
    }
    if(rusage) {
        err = valid_userptr_write(curr_task->mm, rusage, sizeof(struct rusage));
        if(err)
            return (pid_t)err;
    }
    return do_wait4(pid, status, options, rusage);
}

int sys_nanosleep(const struct timespec *req, struct timespec *rem) {
    int err;

    err = valid_userptr_read(curr_task->mm, req, sizeof(struct timespec));
    if(err)
        return (pid_t)err;
    /* rem is filled in only if it is not NULL */
    if(rem) {
        err = valid_userptr_write(curr_task->mm, rem, sizeof(struct timespec));
        if(err)
            return (pid_t)err;
    }
    return do_nanosleep(req, rem);
}

unsigned int sys_alarm(unsigned int seconds)  {
    return 0;
}

char *sys_getcwd(char *buf, size_t size) {
    int err;
    if(!buf)
        return (void *)-EFAULT;
    err = valid_userptr_write(curr_task->mm, buf, size);
    if(err)
        return (void *)(int64_t)err;
    return do_getcwd(buf, size);
}

int sys_chdir(const char *path) {
    if(!path)
        return -EFAULT;

    /* TODO: validate unbounded pointer ???? */
    return do_chdir(path);
}

int sys_open(const char *pathname, int flags, mode_t mode) {
    /* TODO: validate unbounded pointer ???? */
    return do_open(pathname, flags, mode);
}

ssize_t sys_read(int fd, void *buf, size_t count) {
    int err;
    if(!buf)
        return -EFAULT;
    err = valid_userptr_write(curr_task->mm, buf, count);
    if(err)
        return err;
    return do_read(fd, buf, count);
}

ssize_t sys_write(int fd, const void *buf, size_t count) {
    int err;
    if(!buf)
        return -EFAULT;
    err = valid_userptr_read(curr_task->mm, buf, count);
    if(err)
        return err;
    return do_write(fd, buf, count);
}

off_t sys_lseek(int fd, off_t offset, int whence) {
    return do_lseek(fd, offset, whence);
}

int sys_close(int fd) {
    return do_close(fd);
}

int sys_pipe(int *pipefd) {
    int err;
    if(!pipefd)
        return -EFAULT;
    err = valid_userptr_write(curr_task->mm, pipefd, 2 * sizeof(int));
    if(err)
        return err;
    return do_pipe(pipefd);
}

int sys_dup(int oldfd) {
    return do_dup(oldfd);
}

int sys_dup2(int oldfd, int newfd) {
    return do_dup2(oldfd, newfd);
}

/**
 * @dirp: pointer to count bytes
 * @count: the number of bytes in dirp
 */
int sys_getdents(unsigned int fd, struct dirent *dirp, unsigned int count) {
    int err;
    if(!dirp)
        return -EFAULT;
    err = valid_userptr_write(curr_task->mm, dirp, count);
    if(err)
        return err;
    return do_getdents(fd, dirp, count);
}

int sys_uname(struct utsname *buf) {
    int err;
    if(!buf)
        return -EFAULT;
    err = valid_userptr_write(curr_task->mm, buf, sizeof(struct utsname));
    if(err)
        return err;
    return do_uname(buf);
}

void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd,
               off_t offset) {
    /* TODO: validate user pointer */
    return do_mmap(addr,length, prot, flags, fd, offset);
}

int sys_munmap(void *addr, size_t length) {
    /* TODO: validate user pointer */
    return do_munmap(addr, length);
}

int64_t syscall_dispatch(int64_t a1, int64_t a2, int64_t a3,
                         int64_t a4, int64_t a5, int64_t a6, int64_t sysnum) {
    int64_t rv;
    debug("Doing a syscall: %d\n", sysnum);
    switch(sysnum) {
        case SYS_read:
            rv = sys_read((int)a1, (void *)a2, (size_t)a3);
            break;
        case SYS_write:
            rv = sys_write((int)a1, (void *)a2, (size_t)a3);
            break;
        case SYS_open:
            rv = sys_open((const char *)a1, (int)a2, (mode_t)0);
            break;
        case SYS_close:
            rv = sys_close((int)a1);
            break;
        case SYS_lseek:
            rv = sys_lseek((int)a1, (off_t)a2, (int)a3);
            break;
        case SYS_mmap:
            rv = (int64_t)sys_mmap((void *)a1, (size_t)a2, (int)a3, (int)a4,
                                   (int)a5, (off_t)a6);
            break;
        case SYS_munmap:
            rv = sys_munmap((void *)a1, (size_t)a2);
            break;
        case SYS_brk:
            rv = sys_brk((void *)a1);
            break;
        case SYS_pipe:
            rv = sys_pipe((int *)a1);
            break;
        case SYS_dup:
            rv = sys_dup((int)a1);
            break;
        case SYS_dup2:
            rv = sys_dup2((int)a1, (int)a2);
            break;
        case SYS_nanosleep:
            rv = sys_nanosleep((const struct timespec *)a1, (struct timespec *)a2);
            break;
        case SYS_alarm:
            rv = sys_alarm((unsigned int)a1);
            break;
        case SYS_getpid:
            rv = sys_getpid();
            break;
        case SYS_fork:
            rv = sys_fork();
            break;
        case SYS_execve:
            rv = sys_execve((const char *)a1, (const char **)a2, (const char **)a3);
            break;
        case SYS_exit:
            sys_exit((int)a1); /* Exit should not return to the user */
            rv = -EAGAIN;      /* try again :P */
            break;
        case SYS_wait4:
            rv = sys_wait4((pid_t)a1, (int *)a2, (int)a3, (struct rusage *)a4);
            break;
        case SYS_uname:
            rv = sys_uname((struct utsname *)a1);
            break;
        case SYS_getdents:
            rv = sys_getdents((unsigned int)a1, (struct dirent *)a2, (unsigned int)a3);
            break;
        case SYS_getcwd:
            rv = (int64_t)sys_getcwd((char *)a1, (size_t)a2);
            break;
        case SYS_chdir:
            rv = sys_chdir((const char *)a1);
            break;
        case SYS_getppid:
            rv = sys_getppid();
            break;
        default: rv = -ENOSYS;
    }
    debug("Did a syscall: %d\n", sysnum);
    return rv;
}
