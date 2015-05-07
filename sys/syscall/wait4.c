#include <sbunix/syscall.h>
#include <sys/wait.h> /* W* defines */

/**
 * TODO: this
 */
pid_t do_wait4(pid_t pid, int *status, int options, struct rusage *rusage) {
    return (pid_t)-ECHILD;
}
