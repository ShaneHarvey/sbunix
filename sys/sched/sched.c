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
        .kernel_rsp = 0, /* Will be set on first call to schedule */
        .mm = &kernel_mm,
        .next_task = &kernel_task,
        .prev_task = &kernel_task,
        .next_rq = NULL,
        .cmdline = "kmain"
};
/* The currently running task */
struct task_struct *curr_task = &kernel_task;
/* The last (previous) task to run, may need to be reaped */
struct task_struct *last_task = NULL;

struct queue run_queue = {
        .num_switches = 0,
        .tasks = NULL,
};

struct queue just_ran_queue = {
        .num_switches = 0,
        .tasks = NULL,

};

struct queue block_queue = {
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
struct task_struct *ktask_create(void (*start)(void), char *name) {
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

    /* TODO: maybe we steal our parent's foreground status */
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
 * Free the task. Only called when removed from its queue.
 */
void task_destroy(struct task_struct *task) {
    int i;
    mm_destroy(task->mm);

    if(task->next_task)
        task->next_task->prev_task = task->prev_task;
    if(task->prev_task)
        task->prev_task->next_task = task->next_task;

    free_page(ALIGN_DOWN(task->kernel_rsp, PAGE_SIZE));

    /* Close any open files */
    for(i = 0; i < TASK_FILES_MAX; i++) {
        struct file *fp = task->files[i];
        if(fp) {
            fp->f_op->close(fp);
            task->files[i] = NULL;
        }
    }

    /* TODO: reference will be kfree'd when parent calls wait() */
}

/**
 * Set the task cmdline to the string cmdline
 */
void task_set_cmdline(struct task_struct *task, char *cmdline) {
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
    } else {
        kpanic("Don't know which queue to put task into: state=%d\n", task->state);
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
 * Block the current task, this is called inside a system call so interrupts
 * are disabled
 */
void task_block(void) {
    curr_task->state = TASK_BLOCKED;
    schedule();
}

/**
 * Unblock the task which was waiting for the terminal.
 */
void task_unblock_foreground(void) {
    struct task_struct *task = block_queue.tasks;
    if(!task)
        return;

    for(;task != NULL; task = task->next_rq) {
        if(task->foreground && !task_sleeping(task)) {
            /* It is the foreground, and not just sleeping */
            task->state = TASK_RUNNABLE;
            rr_queue_remove(&block_queue, task);
            rr_queue_add(&run_queue, task); /* We want this to run on next schedule() */
        }
    }
}

/**
 * @return: 1 if sleeping, 0 if not sleeping
 */
int task_sleeping(struct task_struct *task) {
    return task->sleepts.tv_nsec || task->sleepts.tv_sec;
}

/**
 * Reset the timeslice.
 */
void reset_timeslice(struct task_struct *task) {
    /* TODO */
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
}

/**
 * Return the next PID to use.
 */
pid_t get_next_pid(void) {
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
    /* Update the kernel stack in the tss */
    tss.rsp0 = curr_task->kernel_rsp;
    syscall_kernel_rsp = curr_task->kernel_rsp;
    /* todo: ltr or ldtr to load the TSS again? */

    /* Clean up the previous task */
//    debug("Switched from %s --> %s\n", last_task->cmdline, curr_task->cmdline);
    if(last_task->state == TASK_DEAD) {
        task_destroy(last_task);
    } else {
        queue_add_by_state(last_task);
    }
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

static void funX(void);
static void funY(void);

static struct task_struct *taskX, *taskY;

void debug_task(struct task_struct *task) {
    debug("type=%d, state=%d, first_switch=%d, foreground=%d\n",
          task->type, task->state, task->first_switch, task->foreground);
}


void scheduler_test(void) {
    int x = 1;
    debug("START OF SCHEDULER TEST\n");
    freemem_report();
//    halt_loop("");
    taskX = ktask_create(funX, "TaskX");
    debug_task(taskX);
    taskY = ktask_create(funY, "TaskY");
    debug_task(taskY);
    while(x++ < 10) {
        /* This is the main task (the kernel_task) */
        __asm__ __volatile__ ("hlt;");
        debug_queues();
        schedule();
    }
    freemem_report();
    kpanic("END OF SCHEDULER TEST\n");
}

static void funX(void) {
    int x = 0;
    printk("X\n");
    while(1) {
        x++;
        if(x == 5)
            kill_curr_task(0);

        debug_task(taskX);
        if(taskX != curr_task) {
            kpanic("taskX not the curr_task!\n");
        }
        debug_queues();
        schedule();
    }
}

static void funY(void) {
    int x = 0;
    printk("Y\n");
    while(1) {
        x++;
        if(x == 5)
            kill_curr_task(0);
        debug_task(taskY);
        if(taskY != curr_task) {
            kpanic("taskY not the curr_task!\n");
        }
        debug_queues();
        schedule();
    }
}