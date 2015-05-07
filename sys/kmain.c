#include "kmain.h"
#include "sched/roundrobin.h"
#include <sbunix/sbunix.h>
#include <sbunix/console.h>

#include "test/test.h"

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    clear_console();
    printk("*** Welcome to SBUnix ***\n");
    /* IRQs off in kernel */
    cli();

    ktask_create(test_exec, "TestExec");

    /* idle task */
    while(1){
        schedule();
        __asm__ __volatile__("sti;hlt;");
    }

    kpanic("\nReturned to kmain!!!\n");
}
