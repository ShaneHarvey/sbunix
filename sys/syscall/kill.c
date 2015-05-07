#include <sbunix/syscall.h>
#include <sbunix/sched.h>
#include <sys/signal.h> /* SIG* defines */

/**
 * pid > 0, send sig to pid
 * pid = 0, send sig to every task in process group of caller (does this mean children?)
 * pid = -1, send sig to all tasks with permission to signal
 * pid < -1, send sig to every task in process group == -pid
 *
 * sig = 0, still perform error checks, but send no signal.
 */
int do_kill(pid_t pid, int sig) {
    struct task_struct *task = kernel_task.next_task;
    int sendtoall = 0;
    int sigs_sent = 0;

    if (pid == 0 || pid == -1){
        sendtoall = 1;
    } else if (pid < -1){
        /* We have no process groups, so just return success */
        /*pid = -pid;*/
        return 0;
    }

    /* Send to all or */
    for(; task != &kernel_task; task = task->next_task) {
        /* Don't signal yourself (yet), never signal kernel tasks, and
         * never signal the init task either */
        if(task == curr_task || task->type == TASK_KERN || task->pid == 1)
            continue;

        /* Does the pid match? Or we're sending to everyone */
        if(task->pid == pid || sendtoall) {
            send_signal(task, sig);
            sigs_sent++;
            if(!sendtoall)
                break; /* They only wanted to send 1 */
        }
    }

    if(sigs_sent == 0)
        return -ESRCH;

    return 0;
}
