#include "kmain.h"
#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/mm/pt.h>

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
    {
        /* Quick test of map_page */
        uint64_t phys_page = get_zero_page();
        uint64_t va = 0x800000;
        printf("Attempting to map physa 0x%lx to va 0x%lx\n", phys_page, va);
        if(map_page(va, phys_page, PFLAG_RW))
            printf("Success: Mapped virtual 0x%lx to physical 0x%lx, data 0x%lx\n", va, phys_page, *(uint64_t *)va);
        else
            kpanic("Failed to map!\n");
    }
}
