#include <syscall.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

int brk(void *addr) {
    /* following what glibc does: 0 on success, -1 on failure */
    /* Linux brk returns new_break on success, unchanged_break on failure */
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

int nanosleep(const struct timespec *req, struct timespec *rem) {
    return (int) syscall_2(SYS_nanosleep, (uint64_t)req, (uint64_t)rem);
}

unsigned int alarm(unsigned int seconds) {
    return (unsigned int) syscall_1(SYS_alarm, (uint64_t)seconds);
}

char *getcwd(char *buf, size_t size) {
    return (char*) syscall_2(SYS_getcwd, (uint64_t)buf, (uint64_t)size);
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