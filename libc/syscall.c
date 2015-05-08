#include <syscall.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>

#define SYSCALL_ERROR_RETURN(rv) do { \
        if(rv < 0 && rv > -4096) {    \
            errno = (int)-rv;         \
            rv = -1;                  \
        }                             \
        return rv;                    \
    } while(0)

/* function arguments  rdi, rsi, rdx, rcx, r8, r9
 * function returns    rax, rdx

 * syscall number      rax
 * arguments           rdi, rsi, rdx, r10, r8, r9.
 * syscall return      rax
 */
int64_t syscall_0(int64_t n) {
    int64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "syscall;"
        "movq %%rax, %0;"
        : "=g"(rv)  /* output */
        :"g"(n)     /* input */
        : "%rax"    /* clobbered registers */
    );
    SYSCALL_ERROR_RETURN(rv);
}

int64_t syscall_1(int64_t n, int64_t a1) {
    int64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=g"(rv)
        :"g"(n), "g"(a1)
        : "%rax", "%rdi"
    );
    SYSCALL_ERROR_RETURN(rv);
}

int64_t syscall_2(int64_t n, int64_t a1, int64_t a2) {
    int64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=g"(rv)
        :"g"(n), "g"(a1), "g"(a2)
        : "%rax", "%rdi", "%rsi"
    );
    SYSCALL_ERROR_RETURN(rv);
}

int64_t syscall_3(int64_t n, int64_t a1, int64_t a2, int64_t a3) {
    int64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "syscall;"
        "movq %%rax, %0;"
        : "=g"(rv)
        :"g"(n), "g"(a1), "g"(a2), "g"(a3)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );
    SYSCALL_ERROR_RETURN(rv);
}

int64_t syscall_4(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4) {
    int64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "movq %5, %%r10;"
        "syscall;"
        "movq %%rax, %0;"
        : "=g"(rv)
        :"g"(n), "g"(a1), "g"(a2), "g"(a3), "g"(a4)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10"
    );
    SYSCALL_ERROR_RETURN(rv);
}

int64_t syscall_5(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                  int64_t a5) {
    int64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "movq %5, %%r10;"
        "movq %6, %%r8;"
        "syscall;"
        "movq %%rax, %0;"
        : "=g"(rv)
        :"g"(n), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8"
    );
    SYSCALL_ERROR_RETURN(rv);
}

int64_t syscall_6(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                  int64_t a5, int64_t a6) {
    int64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "movq %5, %%r10;"
        "movq %6, %%r8;"
        "movq %7, %%r9;"
        "syscall;"
        "movq %%rax, %0;"
        : "=g"(rv)
        :"g"(n), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5), "g"(a6)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "%r9"
    );
    SYSCALL_ERROR_RETURN(rv);
}


/* System call wrappers */

void exit(int status) {
    syscall_1(SYS_exit,  (uint64_t)status);
}

int brk(void *addr) {
    /**
    * Calls the real Linux syscall, which returns:
    *   new break     -- on success
    *   current break -- on failure
    */
    void *newaddr = (void*)syscall_1(SYS_brk, (uint64_t)addr);
    if(newaddr == addr)
        return 0;
    return -1;
}

pid_t fork(void) {
    return (pid_t) syscall_0(SYS_fork);
}

pid_t getpid(void) {
    return (pid_t) syscall_0(SYS_getpid);
}

pid_t getppid(void) {
    return (pid_t) syscall_0(SYS_getppid);
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
    return (int) syscall_3(SYS_execve, (uint64_t)filename, (uint64_t)argv,
            (uint64_t)envp);
}

pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage){
    return (pid_t) syscall_4(SYS_wait4, (uint64_t)pid, (uint64_t)status,
            (uint64_t)options, (uint64_t)rusage);
}

int kill(pid_t pid, int sig) {
    return (int) syscall_2(SYS_kill, (uint64_t)pid, (uint64_t)sig);
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    return (int) syscall_2(SYS_nanosleep, (uint64_t)req, (uint64_t)rem);
}

unsigned int alarm(unsigned int seconds) {
    return (unsigned int) syscall_1(SYS_alarm, (uint64_t)seconds);
}

char *getcwd(char *buf, size_t size) {
    if(syscall_2(SYS_getcwd, (uint64_t)buf, (uint64_t)size) < 0)
        return NULL;
    return buf;
}

int chdir(const char *path) {
    return (int) syscall_1(SYS_chdir, (uint64_t)path);
}

int open(const char *pathname, int flags) {
    return (int) syscall_2(SYS_open, (uint64_t)pathname, (uint64_t)flags);
}

ssize_t read(int fd, void *buf, size_t count) {
    return (ssize_t) syscall_3(SYS_read, (uint64_t)fd, (uint64_t)buf,
            (uint64_t)count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return (ssize_t) syscall_3(SYS_write, (uint64_t)fd, (uint64_t)buf,
            (uint64_t)count);
}

off_t lseek(int fildes, off_t offset, int whence) {
    return (off_t) syscall_3(SYS_lseek, (uint64_t)fildes, (uint64_t)offset,
            (uint64_t)whence);
}

int close(int fd) {
    return (int) syscall_1(SYS_close, (uint64_t)fd);
}

int pipe(int filedes[2]) {
    return (int) syscall_1(SYS_pipe, (uint64_t)filedes);
}

int dup(int oldfd) {
    return (int) syscall_1(SYS_dup, (uint64_t)oldfd);
}

int dup2(int oldfd, int newfd) {
    return (int) syscall_2(SYS_dup2, (uint64_t)oldfd, (uint64_t)newfd);
}

int getdents(unsigned int fd, struct dirent *dirp, unsigned int count) {
    return (int) syscall_3(SYS_getdents, (uint64_t)fd, (uint64_t)dirp,
            (uint64_t)count);
}

int uname(struct utsname *buf) {
    return (int) syscall_1(SYS_uname, (uint64_t)buf);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
        off_t offset) {
    return (void *) syscall_6(SYS_mmap, (uint64_t)addr, (uint64_t)length,
            (uint64_t)prot, (uint64_t)flags, (uint64_t)fd, (uint64_t)offset);
}

int munmap(void *addr, size_t length) {
    return (int) syscall_2(SYS_munmap, (uint64_t)addr, (uint64_t)length);
}

/* procbuf is a buffer that will contain struct proc_struct's */
ssize_t getprocs(void *procbuf, size_t length) {
    return (int) syscall_2(SYS_getprocs, (uint64_t)procbuf, (uint64_t)length);
}
