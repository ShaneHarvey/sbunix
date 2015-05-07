#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/syscall.h>

/* from syscall_entry.s */
extern void child_ret_from_fork(void);

/**
 * TODO: fixme: this is the only thing that breaks without -01 optimization
 *
 * Also, I'm not so sure it works in the first place...
 */
pid_t do_fork(void) {
    struct task_struct *child;

    child = fork_curr_task();
    if(!child)
        return (pid_t)-ENOMEM;

    /* We want to retq to child_ret_from_fork */
    child->first_switch = 1;

    /* -(16 * 8), for 16 popq's after child_ret_from_fork
     * -8, for retq pop */
    child->kernel_rsp = ALIGN_UP(child->kernel_rsp, PAGE_SIZE) - 16 - 128 - 8;
    *(uint64_t *)child->kernel_rsp = (uint64_t)child_ret_from_fork;

    schedule();
    debug("PARENT RETURNED FROM SCHEDULE: returning child pid %d\n", child->pid);
    return child->pid;
}
