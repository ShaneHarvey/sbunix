#include <sbunix/syscall.h>
#include <sbunix/sched.h>
#include <sys/getprocs.h>
#include <sbunix/string.h>

/**
 * Doesn't show the idle task.
 * Args have been error checked.
 * @procbuf: buffer into which proc_struct's will be stored
 * @length:  bytes in the buffer
 */
ssize_t do_getprocs(void *procbuf, size_t length) {
    struct task_struct *task;
    ssize_t wrote = 0;

    task = kernel_task.next_task;
    /* For all tasks on the system */
    for(; task != &kernel_task; task = task->next_task) {
        size_t cmdlen;
        /* Don't show the kernel tasks */
        if(task->type == TASK_KERN)
            continue;

        cmdlen = strnlen(task->cmdline, TASK_CMDLINE_MAX);
        if(wrote + sizeof(pid_t) + cmdlen + 1 > length) {
            return wrote;  /* Buffer can't fit the next process */
        }

        /* procbuf can fit */
        *(pid_t*)(wrote + (char*)procbuf) = task->pid;
        wrote += sizeof(pid_t);
        strlcpy((wrote + (char*)procbuf),task->cmdline, cmdlen + 1);
        wrote += cmdlen + 1;
    }
    return wrote;
}
