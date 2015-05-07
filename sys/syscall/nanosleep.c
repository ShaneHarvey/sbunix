#include <sbunix/syscall.h>
#include <sbunix/sched.h>
#include <sbunix/sbunix.h>

/**
 * Sleep with nanosecond granularity (limited by PIT frequency)
 *
 * @req: the requested amount of time to sleep
 * @rem: filled in will zeros if it is not NULL. Since we don't have signals
 * a process always sleep for the entire request
 */
int do_nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!req)
        return -EFAULT;

    if(req->tv_sec < 0 || req->tv_nsec < 0 || req->tv_nsec > 999999999L)
        return -EINVAL;

    if(req->tv_sec || req->tv_nsec) {
        curr_task->sleepts.tv_sec = req->tv_sec;
        curr_task->sleepts.tv_nsec = req->tv_nsec;
        curr_task->state = TASK_SLEEPING;
    }
    schedule();

    printk("Task %s: waking up from sleep!\n", curr_task->cmdline);
    if(rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    return 0;
}
