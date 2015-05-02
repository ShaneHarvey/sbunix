#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/mm/vmm.h>
#include <sbunix/mm/pt.h>
#include <sbunix/mm/align.h>
#include <sbunix/string.h>
#include <sbunix/gdt.h>

/* All kernel tasks use this mm_struct */
struct mm_struct kernel_mm = {0};
/* This task is associated with kmain(), the kernel at startup */
struct task_struct kernel_task = {
        .type = TASK_KERN,
        .state = TASK_RUNNABLE,
        .flags = 0,
        .kernel_rsp = 0, /* Will be set on first call to schedule */
        .mm = &kernel_mm,
        .next_task = &kernel_task,
        .prev_task = &kernel_task,
        .next_rq = &kernel_task,
        .prev_rq =  &kernel_task
};
/* The currently running task */
struct task_struct *curr_task = &kernel_task;
/* The last (previous) task to run, may need to be reaped */
struct task_struct *last_task = NULL;

struct rq run_queue = {
        .num_switches = 0,
        .tasks = NULL,
};

/* Private functions */
void run_queue_add(struct rq *queue, struct task_struct *task);
void task_list_add(struct task_struct *task);
void task_add_new(struct task_struct *task);


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

    task = kmalloc(sizeof(struct task_struct));
    if(!task)
        goto out_stack;

    task->type = TASK_KERN;
    task->state = TASK_RUNNABLE;
    /* Put the start function on the stack for switch_to  */
    task->flags = TASK_FIRST_SWITCH;
    stack[511] = (uint64_t)start;
    task->kernel_rsp = (uint64_t)&stack[511];
    task->mm = &kernel_mm;
    kernel_mm.mm_count++;
    task->mm = NULL;
    task_set_cmdline(task, name);

    task_add_new(task); /* add to run queue and task list */
    return task;

out_stack:
    free_page((uint64_t)stack);
    return NULL;
}

/**
 * Free the task.
 */
void task_destroy(struct task_struct *task) {
    mm_destroy(task->mm);

    if(task->next_task)
        task->next_task->prev_task = task->prev_task;
    if(task->prev_task)
        task->prev_task->next_task = task->next_task;

    free_page(ALIGN_DOWN(task->kernel_rsp, PAGE_SIZE));
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
 * Add a task to the linked list of tasks in queue.
 */
void run_queue_add(struct rq *queue, struct task_struct *task) {
    if(!queue || !task)
        return;

    if(!queue->tasks) {
        /* Add the first element of the queue */
        task->next_rq = task;
        task->prev_rq = task;
        queue->tasks = task;
    } else {
        /* Insert task behind the current (at the end of the queue) */
        struct task_struct *hd = queue->tasks;
        hd->prev_rq->next_rq = task;
        task->prev_rq = hd->prev_rq;
        hd->prev_rq = task;
        task->next_rq = hd;
    }
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
void task_queue_add(struct task_struct *task) {
    if(task->state == TASK_RUNNABLE) {
        run_queue_add(&run_queue, task);
    }
}

/**
 * Remove task from its queue.
 */
void task_queue_remove(struct task_struct *task) {
    if(task->next_rq)
        task->next_rq->prev_rq = task->prev_rq;
    if(task->prev_rq)
        task->prev_rq->next_rq = task->next_rq;
}

/**
 * Add a newly created task to the system.
 * Adds it to the list of all tasks and to the appropriate queue.
 */
void task_add_new(struct task_struct *task) {
    if(!task)
        return;

    task_list_add(task);
    task_queue_add(task);
}

/**
 * Pick the highest priority task to run.
 */
struct task_struct *pick_next_task(void) {
    struct task_struct *task;
    task = run_queue.tasks;
    if(!task)
        return curr_task;
    run_queue.tasks = run_queue.tasks->next_task;
    task_queue_remove(task);
    return task;
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
inline void context_switch(struct task_struct *prev, struct task_struct *next) {
    struct mm_struct *mm, *prev_mm;
    mm = next->mm;
    prev_mm = prev->mm;

    if(next->type != TASK_KERN) /* All kernel tasks can use any mm_struct */
        switch_mm(prev_mm, mm);

    /* TODO: verify that switch_to() is probably broken */
    switch_to(prev, next);
    /* next is now the current task */
}

/**
 * Switch out the current task for the next task to run.
 */
void schedule(void) {
    struct task_struct *prev, *next;

    /* Assuming atomicity */
    prev = curr_task;
    next = pick_next_task();

    if(prev != next) {
        run_queue.num_switches++;
        curr_task = next;
        last_task = prev; /* the last_task to run is the "prev" */

        context_switch(prev, next);

        /* change the kernel stack in the tss */
        tss.rsp0 = curr_task->kernel_rsp;
        /* todo: ltr or ldtr to load the TSS again? */

        if(last_task->state == TASK_DEAD) {
            task_destroy(last_task);
        } else {
            task_queue_add(last_task);
        }
    }

}