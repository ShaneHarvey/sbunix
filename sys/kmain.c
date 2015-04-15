#include "kmain.h"
#include <sbunix/sbunix.h>
#include <sbunix/sched.h>

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    struct task_struct *cur, *test;

    cur = (struct task_struct *)kmalloc(sizeof(struct task_struct));
    test = (struct task_struct *)kmalloc(sizeof(struct task_struct));

    cur->kernel_rsp = read_rsp();
    cur->prev_task = test;
    cur->next_task = test;

    test->next_task = cur;
    test->prev_task = cur;
    /* I don't know what I'm doing */
}
