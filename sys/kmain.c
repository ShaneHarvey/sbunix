#include "kmain.h"
#include "sched/roundrobin.h"
#include <sbunix/sbunix.h>
#include <sbunix/console.h>

#include "test/test.h"

void run_init(void);

/* Initialized by kmain, it is PID 1 and /bin/init */
struct task_struct *init_task;

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    clear_console();
    printk("*** Welcome to SBUnix ***\n");
    /* IRQs off in kernel */
    cli();

    init_task = ktask_create(run_init, "[init]");

    /* idle task */
    while(1){
        schedule();
        __asm__ __volatile__("sti;hlt;");
    }

    kpanic("\nReturned to kmain!!!\n");
}

void run_init(void) {
    int err;

    /* Init stdout/stdin/stderr */
    err = task_files_init(curr_task);
    if(err)
        kpanic("init: failed to create terminal: %s\n", strerror(-err));

    /* Start the init process, this should not return */
    err = do_execve("/bin/init", NULL, NULL);
    if(err)
        kpanic("init: '/bin/init' failed: %s\n", strerror(-err));

    /* cleanup if do_execve fails */
    kill_curr_task(0);
}
