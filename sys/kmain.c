#include "kmain.h"
#include "sched/roundrobin.h"
#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/syscall.h>
#include <sbunix/string.h>
#include <sbunix/console.h>
#include <sbunix/fs/terminal.h>

#include "test/test.h"

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    clear_console();
    printk("*** Welcome to SBUnix ***\n");

    task_files_init(&kernel_task);
    ktask_create(test_exec, "TestExec");


    while(1){
        schedule();
    }

    kpanic("\nReturned to kmain!!!\n");
}
