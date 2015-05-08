#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/mm/vmm.h>
#include <sbunix/mm/pt.h>
#include <sbunix/mm/align.h>
#include <sbunix/string.h>
#include <sbunix/gdt.h>
#include <sbunix/interrupt/pit.h>
#include <sbunix/fs/terminal.h>
#include <errno.h>
#include "roundrobin.h"
#include "../syscall/syscall_dispatch.h"

/* All kernel tasks use this mm_struct */
struct mm_struct kernel_mm = {0};
/* This task is associated with kmain(), the kernel at startup */
struct task_struct kernel_task = {
        .type = TASK_KERN,
        .state = TASK_RUNNABLE,
        .first_switch = 0,
        .foreground = 1, /* can read from the terminal */
        .in_syscall = 0,
        .timeslice = 0,  /* does not have timeslice */
        .sleepts = {0, 0},
        .pid = 0,
        .exit_code = 0,
        .blocked_on = NULL,
        .kernel_rsp = 0, /* Will be set on first call to schedule */
        .mm = &kernel_mm,
        .next_task = &kernel_task,
        .prev_task = &kernel_task,
        .next_rq = NULL,
        .parent = NULL,
        .chld = NULL,
        .sib = NULL,
        .files = {0},
        .cmdline = "kmain",
        .cwd = "/",
};
/* The currently running task */
struct task_struct *curr_task = &kernel_task;
/* The last (previous) task to run, may need to be reaped */
static struct task_struct *last_task = NULL;

/* Holds a singly linked list of tasks in the state TASK_RUNNABLE.
 * When depleted it grabs the list from the just_ran_queue. */
struct queue run_queue = {
        .num_switches = 0,
        .tasks = NULL,
};

/* Holds a singly linked list of tasks in the state TASK_RUNNABLE */
struct queue just_ran_queue = {
        .num_switches = 0,
        .tasks = NULL,

};

/* Hold's tasks in the state TASK_BLOCKED, blocked on terminals or pipes */
struct queue block_queue = {
        .num_switches = 0,
        .tasks = NULL,
};

/* Hold's tasks in the state TASK_SLEEPING, from call to nanosleep(2) */
struct queue sleep_queue = {
        .num_switches = 0,
        .tasks = NULL,
};

/* Hold's tasks in the state TASK_WAITING, from call to waitpid(2) */
struct queue wait_queue = {
        .num_switches = 0,
        .tasks = NULL,
};

/* Global next PID to give out, monotonically increasing */
static pid_t next_pid = 1;

/* Private functions */
static void task_list_add(struct task_struct *task);
static void task_add_new(struct task_struct *task);
static void add_child(struct task_struct *parent, struct task_struct *chld);


void scheduler_init(void) {
    /* set the kernel's page table to the initial pagetable */
    kernel_mm.pml4 = kernel_pt;
    kernel_mm.mm_count++; /* plus 1 for the kernel itself? */
}

void scheduler_start(void) {
    /* currently no use */
}

/**
 * Create a kernel task. It's state will be TASK_RUNNABLE and
 * the type will be TASK_KERN.
 * @start: function the task will start at, no arguments and returns void
 */
struct task_struct *ktask_create(void (*start)(void), const char *name) {
    struct task_struct *task;
    uint64_t *stack;

    stack = (uint64_t *)get_free_page(0);
    if(!stack)
        return NULL;

    task = kmalloc(sizeof(*task));
    if(!task)
        goto out_stack;

    memset(task, 0, sizeof(*task));

    task->type = TASK_KERN;
    task->state = TASK_RUNNABLE;
    /* Put the start function on the stack for switch_to  */
    task->first_switch = 1;
    task->foreground = 1; /* all kernel threads can read input */
    stack[510] = (uint64_t)start;
    task->kernel_rsp = (uint64_t)&stack[510];
    task->mm = &kernel_mm;
    kernel_mm.mm_count++;
    task->pid = get_next_pid();
    task_set_cmdline(task, name);
    strcpy(task->cwd, "/"); /* set cwd to root for ktasks */
    task->timeslice = TIMESLICE_BASE;

    task_add_new(task); /* add to run queue and task list */
    return task;

out_stack:
    free_page((uint64_t)stack);
    return NULL;
}

/**
 * Return a copy of the current task.
 */
struct task_struct *fork_curr_task(void) {
    struct task_struct *task;
    uint64_t *kstack, *curr_kstack;
    int i;

    kstack = (uint64_t *)get_free_page(0);
    if(!kstack)
        return NULL;

    task = kmalloc(sizeof(*task));
    if(!task)
        goto out_stack;

    /* Half the remaining timeslice (split between parent and child) */
    curr_task->timeslice >>= 1;

    memcpy(task, curr_task, sizeof(*task));     /* Exact copy of parent */

    /* deep copy the current mm */
    task->mm = mm_deep_copy();
    if(task->mm == NULL)
        goto out_task;

    /* Copy the curr_task's kstack */
    curr_kstack = (uint64_t *)ALIGN_DOWN(read_rsp(), PAGE_SIZE);
    memcpy(kstack, curr_kstack, PAGE_SIZE);
    task->kernel_rsp = (uint64_t)&kstack[510];  /* new kernel stack */
    task->pid = get_next_pid();                 /* new pid */
    task->parent = curr_task;                   /* new parent */
    task->chld = task->sib = NULL;              /* no children/siblings yet */
    task->next_task = task->prev_task = task->next_rq = NULL;

    /* Increment reference counts on any open files */
    for(i = 0; i < TASK_FILES_MAX; i++) {
        struct file *fp = task->files[i];
        if(fp) {
            fp->f_count++;
        }
    }

    /* Add this new child to the parent */
    add_child(curr_task, task);

    /* TODO: Here we steal our parent's foreground status */
    curr_task->foreground = 0;/* change to 1; to let all tasks read */

    task_add_new(task); /* add to run queue and task list */

    return task;
out_task:
    kfree(task);
out_stack:
    free_page((uint64_t)kstack);
    return NULL;
}

/**
 * Free the task's kernel stack, memory context, and close open files.
 * NOTE: Only called when removed from its queue.
 * Leaves the task on in the list of all tasks (only remove when a
 * parent calls cleanup_child).
 */
void task_destroy(struct task_struct *task) {
    int i;
    mm_destroy(task->mm);

    free_page(ALIGN_DOWN(task->kernel_rsp, PAGE_SIZE));

    /* Close any open files */
    for(i = 0; i < TASK_FILES_MAX; i++) {
        struct file *fp = task->files[i];
        if(fp) {
            fp->f_op->close(fp);
            task->files[i] = NULL;
        }
    }

    /* Give terminal control back to the parent */
    if(task->foreground && task->parent) {
        task->parent->foreground = 1;
    }
    task->foreground = 0; /* remove the foreground from the dead task */

    /* If we have children give them to init */
    if(task->chld) {
        /* Give all children to init */
        struct task_struct *prevchld, *nextchld;
        for(prevchld = task->chld; prevchld != NULL; prevchld = nextchld) {
            nextchld = prevchld->sib;
            add_child(init_task, prevchld);
        }
    }

    if(task->parent) {
        /* Notify parent of child's termination */
        if(task->parent->state == TASK_WAITING) {
            task_wakeup(&wait_queue, task->parent);
        }
    } else {
        /* We have no parent so add ourself to init */
        add_child(init_task, task);
    }
}

/**
 * Do the final cleanup of a task struct.
 * @return: the exit code of the task
 */
int cleanup_child(struct task_struct *task) {
    int rv;
    if(!task)
        kpanic("Waiting on NULL task\n");

    /* Remove the child from list of all tasks. */
    if(task->next_task)
        task->next_task->prev_task = task->prev_task;
    if(task->prev_task)
        task->prev_task->next_task = task->next_task;

    if(!task->parent)
        kpanic("Reaping a child that has no parent!\n");

    /* Remove task from list of children in parent*/
    if(task->parent->chld == task) {
        /* task was first child */
        task->parent->chld = task->sib;
    } else {
        struct task_struct *prev = task->parent->chld;
        for(; prev->sib != task; prev = prev->sib) {
        }
        prev->sib = task->sib; /* remove task */
    }

    rv = task->exit_code;
    kfree(task);
    return rv;
}

/**
 * Set the task cmdline to the string cmdline
 */
void task_set_cmdline(struct task_struct *task, const char *cmdline) {
    if(!task)
        return;
    strncpy(task->cmdline, cmdline, TASK_CMDLINE_MAX);
    task->cmdline[TASK_CMDLINE_MAX] = 0;
}

/**
 * Add a task to the linked list of ALL tasks in the system.
 */
void task_list_add(struct task_struct *task) {
    if(!task)
        return;

    if(!curr_task) {
        /* Should never be executed */
        task->next_task = task;
        task->prev_task = task;
        curr_task = task;
    } else {
        /* Insert task behind the current */
        curr_task->prev_task->next_task = task;
        task->prev_task = curr_task->prev_task;
        curr_task->prev_task = task;
        task->next_task = curr_task;
    }
}

/**
 * Add task to the correct queue.
 */
void queue_add_by_state(struct task_struct *task) {
//    debug("Adding task: %s\n", task->cmdline);
    if(task->state == TASK_RUNNABLE) {
        rr_queue_add(&just_ran_queue, task);
    } else if(task->state == TASK_BLOCKED) {
        rr_queue_add(&block_queue, task);
    } else if(task->state == TASK_SLEEPING) {
        rr_queue_add(&sleep_queue, task);
    } else if(task->state == TASK_WAITING) {
        rr_queue_add(&wait_queue, task);
    } else {
        kpanic("Don't know which queue to put task into: state=%d\n", task->state);
    }
}

/**
 * Remove task from the queue it resides in.
 */
void queue_remove_by_state(struct task_struct *task) {
    if(task->state == TASK_RUNNABLE) {
        /* Could be in either queue */
        rr_queue_remove(&run_queue, task);
        rr_queue_remove(&just_ran_queue, task);
    } else if(task->state == TASK_BLOCKED) {
        rr_queue_remove(&block_queue, task);
    } else if(task->state == TASK_SLEEPING) {
        rr_queue_remove(&sleep_queue, task);
    } else if(task->state == TASK_WAITING) {
        rr_queue_remove(&wait_queue, task);
    } else {
        kpanic("Don't know which queue to remvoe task from: state=%d\n", task->state);
    }
}


/**
 * Add a newly created task to the system.
 * Adds it to the list of all tasks and to the appropriate queue.
 */
void task_add_new(struct task_struct *task) {
    if(!task)
        return;
    task_list_add(task);
    queue_add_by_state(task);
}

/**
 * Called to end the life of the current task.
 * This function NEVER returns.
 */
void kill_curr_task(int exit_code) {
    curr_task->exit_code = exit_code;
    curr_task->state = TASK_DEAD;
    schedule();
}

/**
 * Kill a task (possibly the current task) with exit code.
 */
void kill_other_task(struct task_struct *task, int exit_code) {
    /* If already dead do nothing */
    if(!task || task->state == TASK_DEAD)
        return;

    /* Let's you kill the current task too */
    if(task == curr_task) {
        kill_curr_task(exit_code);
        return; /* Doesn't return! */
    }

    if(task->type == TASK_KERN && curr_task->type != TASK_KERN) {
        return; /* users can't kill kernel tasks */
    }

    /* Remove from it's queue */
    queue_remove_by_state(task);

    /* Proceed with the kill */
    task->state = TASK_DEAD;
    task->exit_code = exit_code;
    task_destroy(task);
}

/**
 * Signal a task, only the fatal signals are implemented.
 */
void send_signal(struct task_struct *task, int sig) {
    if(sig == 0)
        return;
    /* Fatal signals */
    if(sig == SIGKILL || sig == SIGSEGV || sig == SIGTERM ||
            sig == SIGFPE || sig == SIGINT)
        kill_other_task(task, EXIT_FATALSIG + sig);
}

/**
 * Returns the current task that controls the terminal
 */
struct task_struct *foreground_task(void) {
    struct task_struct *task;
    /* For all tasks on the system */
    for(task = kernel_task.next_task; task != &kernel_task;
        task = task->next_task) {
        if(task->foreground)
            return task;
    }
    debug("NO task is in the foreground!\n");
    return NULL;
}

/**
 * Block the current task, this is called inside a system call so interrupts
 * are disabled
 */
void task_block(void *block_on) {
    curr_task->state = TASK_BLOCKED;
    curr_task->blocked_on = block_on;
    schedule();
}

/**
 * Remove the task from the from queue and add it to the run queue.
 */
void task_wakeup(struct queue *from_queue, struct task_struct *task) {
    task->state = TASK_RUNNABLE;
    task->blocked_on = NULL;
    rr_queue_remove(from_queue, task);
    rr_queue_add(&run_queue, task); /* We want this to run on next schedule() */
}

/**
 * Unblock the first task blocking on blocked_on
 */
void task_unblock(void *blocked_on) {
    struct task_struct *task = block_queue.tasks;
    if(!task)
        return;

    for(;task != NULL; task = task->next_rq) {
        if(blocked_on == task->blocked_on) {
            /* It is the foreground, AND blocking on blocked_on */
            task_wakeup(&block_queue, task);
            /* Wake up ALL, do not break here. Consider blocking on pipes... */
        }
    }
}

/**
 * Unblock the task which was waiting for the terminal.
 */
void task_unblock_foreground(void *blocked_on) {
    struct task_struct *task = block_queue.tasks;
    if(!task)
        return;

    for(;task != NULL; task = task->next_rq) {
        if(task->foreground && (blocked_on == task->blocked_on)) {
            /* It is the foreground, AND blocking on blocked_on */
            task_wakeup(&block_queue, task);
            /* TODO: break; ???? */
        }
    }
}

/**
 * Add chld to parent's list of children.
 */
void add_child(struct task_struct *parent, struct task_struct *chld) {
    if(!parent || !chld)
        return;

    /* Push new child onto the parent's list */
    chld->sib = parent->chld;
    parent->chld = chld;
    chld->parent = parent;
}

/**
 * Return the next PID to use.
 */
pid_t get_next_pid(void) {
    if(next_pid > 2147483647)
        kpanic("Ran out of PIDs, please reboot!");
    return next_pid++;
}

/**
 * Switch to the new mm_struct.
 * @prev: mm_struct of the previously running (current) task
 * @next: mm_struct to switch to
 */
void switch_mm(struct mm_struct *prev, struct mm_struct *next) {
    if(prev != next) {
        write_cr3(next->pml4);
    }
}

/**
 * Switch to the new mm_struct and restore register/stack state to next.
 * @prev: previously running (current) task
 * @next: task to switch to
 */
static inline void __attribute__((always_inline)) context_switch(struct task_struct *prev, struct task_struct *next) {
    struct mm_struct *mm, *prev_mm;
    mm = next->mm;
    prev_mm = prev->mm;

    switch_mm(prev_mm, mm);

    switch_to(prev, next);
    /* next is now the current task */
}

/**
 * Update the TSS with the new kernel stack.
 * Cleanup the last task, destroying or placing on a queue as needed.
 */
static void __attribute__((noinline)) post_context_switch(void) {
    /* Update the kernel stack in the tss (these are always aligned minus 16) */
    tss.rsp0 = ALIGN_UP(curr_task->kernel_rsp, PAGE_SIZE) - 16;
    syscall_kernel_rsp = tss.rsp0;
    /* todo: ltr or ldtr to load the TSS again? */

    /* Clean up the previous task */
//    debug("Switched from %s --> %s\n", last_task->cmdline, curr_task->cmdline);
    /* Do not destroy or add the Idle Task to the run queues */
    if(last_task != &kernel_task) {
        if (last_task->state == TASK_DEAD) {
            task_destroy(last_task);
        } else {
            queue_add_by_state(last_task);
        }
    }

    /* Refill the timeslice */
    reset_timeslice(curr_task);
}

/**
 * Switch out the current task for the next task to run.
 *
 * TODO: schedule MUST always be called with interrupts disabled
 */
void schedule(void) {
    struct task_struct *prev, *next;

    /* Assuming atomicity */
    prev = curr_task;
    next = rr_pick_next_task();

    if(prev != next) {
        run_queue.num_switches++;
        curr_task = next;
        last_task = prev; /* the last_task to run is the "prev" */

        context_switch(prev, next);
        /* WE CAN NOT REFER TO LOCALS AFTER the context_switch */

        post_context_switch();

        /* If this was the first switch then the current stack has no frame for
         * schedule(), on retq we will return to the task's start function for
         * the first time.
         */
        if(curr_task->first_switch) {
            curr_task->first_switch = 0;
            curr_task->kernel_rsp = ALIGN_UP(curr_task->kernel_rsp, PAGE_SIZE) - 16;
            __asm__ __volatile__ ("retq;");
        }
    }

}

/**
 * Init stdin, stdout, stderr in the specified task
 */
int task_files_init(struct task_struct *task) {
    struct file *fp;
    if(!task)
        kpanic("task is NULL!\n");
    if(task->files[0] || task->files[1] || task->files[2])
        kpanic("A file is open during init!!\n");
    fp = term_open();
    if(!fp)
        return -ENOMEM;
    fp->f_count += 2; /* we make 2 "copies" */
    task->files[0] = fp;
    task->files[1] = fp;
    task->files[2] = fp;
    return 0;
}

/**
 * Print some info from a task.
 */
void debug_task(struct task_struct *task) {
    debug("type=%d, state=%d, 1st_switch=%d, fg=%d, cmd=%s\n",
          task->type, task->state, task->first_switch, task->foreground, task->cmdline);
}
