#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/asm.h>

struct task_struct *last_task = NULL;
struct rq run_queue = {
        .num_switches = 0,
        .curr = NULL,
        .tasks = NULL
};

void scheduler_init(void) {

}

void scheduler_start(void) {

}

/**
 * Pick the highest priority task to run.
 */
struct task_struct *pick_next_task(void) {
    struct task_struct *tsk;
    tsk = run_queue.tasks;
    run_queue.tasks = run_queue.tasks->next_task;
    return tsk;
}

/**
 * Switch to the new mm_struct.
 * @prev: mm_struct of the previously running (current) task
 * @next: mm_struct to switch to
 */
void switch_mm(struct mm_struct *prev, struct mm_struct *next) {
    if(prev != next) {
        write_cr3((uint64_t)next->pgd);
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
    prev_mm = prev->active_mm;

    if(mm == NULL) { /* next is a kernel thread */
        next->active_mm = prev_mm;
        prev_mm->mm_count++;
    } else {
        switch_mm(prev_mm, mm);
    }

    if(prev->mm == NULL) {
        prev->active_mm = NULL;
    }
    last_task = prev;
    /* TODO: verify that switch_to() is probably broken */
    switch_to(prev, next);
}

/**
 * Switch out the current task for the next task to run.
 */
void schedule(void) {
    struct task_struct *prev, *next;

    /* Assuming atomicity */
    prev = run_queue.curr;
    next = pick_next_task();

    if(prev != next) {
        run_queue.num_switches++;
        run_queue.curr = next;

        context_switch(prev, next);
    }

}