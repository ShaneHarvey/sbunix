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

    /* -(15 * 8), for 15 popq's after child_ret_from_fork
     * -8, for retq pop */
    child->kernel_rsp = child->kernel_rsp - 120 - 8;
    *(uint64_t *)child->kernel_rsp = (uint64_t)child_ret_from_fork;

    debug("curr_task RSP: %p, child_task RSP: %p\n", read_rsp(), child->kernel_rsp);
    schedule();
    printk("PARENT RETURNED FROM SCHEDULE\n");
    return child->pid;
}
