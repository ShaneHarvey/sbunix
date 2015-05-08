#include <sbunix/syscall.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>
#include <sys/wait.h> /* W* defines */

/**
 *
 * @pid: < -1  TODO meaning  wait  for any child process whose process group ID
 *             is equal to the absolute value of pid.
 *
 *    0 or -1  meaning wait for any child process.
 *
 *        > 0  meaning wait for the child whose process ID is equal to the
 *             value of pid.
 *
 * @status: set to exit status of child, if NULL ignore
 * @options: could only be WNOHANG, or 0
 * @rusage: set usage info (we sent to all 0's), if NULL ignore
 */
pid_t do_wait4(pid_t pid, int *status, int options, struct rusage *rusage) {
    struct task_struct *task;
    int exit_code, childpid, pid_exists = 0;
    /* Wait for any child? */
    int anychild = pid == 0 || pid == -1;

    /* Waiting for children */
    while(1) {

        /* For each child */
        for (task = curr_task->chld; task != NULL; task = task->sib) {
            /* Does a child we were looking for exist? */
            if (task->pid == pid || anychild) {
                pid_exists = 1;
            }
            /* If it's DEAD and we're looking for it */
            if (task->state == TASK_DEAD && pid_exists) {
                childpid = task->pid;
                exit_code = cleanup_child(task);
                if(status)
                    *status = exit_code;
                if(rusage)
                    memset(rusage, 0, sizeof(*rusage)); /* No usage stats */
                return childpid;
            }
        }
        /* Made it through all the children */
        if(!pid_exists) {
            /* If we didn't see a valid pid */
            return (pid_t)-ECHILD;
        }
        if(options & WNOHANG) {
            /* User doesn't want to wait */
            return 0;
        }
        /* Otherwise actually wait! */
        curr_task->state = TASK_WAITING;
        schedule();
    }
}
