#include <syscall.h>
#include <stdlib.h>
#include <sys/wait.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

int brk(void *addr) {
    syscall_1(SYS_brk, (uint64_t)addr);
}

pid_t fork(void) {
    syscall_0(SYS_fork);
}

pid_t getpid(void) {
    syscall_0(SYS_getpid);
}

pid_t getppid(void) {
    syscall_0(SYS_getppid);
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
    syscall_3(SYS_execve, (uint64_t)filename, (uint64_t)argv, (uint64_t)envp);
}

pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage){
    syscall_4(SYS_wait4, (uint64_t)pid, (uint64_t)status, (uint64_t)options,
            (uint64_t)rusage);
}

int nanosleep(const struct timespec *req, struct timespec *rem) {

}

unsigned int alarm(unsigned int seconds) {
    
}

