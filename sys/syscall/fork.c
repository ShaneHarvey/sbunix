#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/syscall.h>


/**
 * TODO: fixme: this is the only thing that breaks without -01 optimization
 *
 * Also, I'm not so sure it works in the first place...
 */
pid_t do_fork(void) {
    struct task_struct *child;
    uint64_t stack_off, curr_stack, child_stack;
    pid_t child_pid;

    child = fork_curr_task();
    if(!child)
        return (pid_t)-ENOMEM;
    /* Magic hack to switch to child task */
    child_pid = child->pid;
    child->first_switch = 1;
    /* offset of the child kstack the same as the current kstack */
    child_stack = ALIGN_UP(child->kernel_rsp, PAGE_SIZE);
    curr_stack = read_rsp();
    stack_off = ALIGN_UP(curr_stack, PAGE_SIZE) - curr_stack;
    child->kernel_rsp = child_stack - stack_off - 8;
    *(uint64_t *)child->kernel_rsp = (uint64_t)&&child_fork_ret;
    debug("curr_task RSP: %p, child_task RSP: %p\n",curr_stack, child->kernel_rsp);
    schedule();
    /* this will never happen, but the label gets optimized out if we don't trick gcc */
    if(child_pid == 0) {
        child_fork_ret: /* Child will retq to this label the first time it gets scheduled */
        __asm__ __volatile__("xorq %rax, %rax; popq %rbx; retq;"); /* TODO: this is stupidly bad */
        return 0;
    }
    return child_pid;
}
