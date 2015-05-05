#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include "roundrobin.h"

struct queue just_ran_queue = {
    .num_switches = 0,
    .tasks = NULL
};

/**
 * Add a task to the front of the list of tasks in queue.
 */
void rr_queue_add(struct queue *queue, struct task_struct *task) {
    if(!queue || !task)
        return;

    task->next_rq = NULL;
    if(!queue->tasks) {
        /* Add the first element of the queue */
        queue->tasks = task;
    } else {
        /* Insert into the end of the queue */
        struct task_struct *prev = queue->tasks;
        for(; prev->next_rq != NULL; prev = prev->next_rq) {
        }
        prev->next_rq = task;
    }
}

/**
 * Search for and remove the given task from the queue.
 * No error if not found.
 */
void rr_queue_remove(struct queue *queue, struct task_struct *task) {
    if(!queue || !queue->tasks || !task)
        return;

    if(queue->tasks == task)
        /* Simple, it was the head of the list. */
        queue->tasks =  queue->tasks->next_rq;
    else {
        struct task_struct *prev = queue->tasks;
        /* Search for task, removing it if found */
        for(; prev->next_rq != NULL; prev = prev->next_rq) {
            if(prev->next_rq == task) {
                prev->next_rq = prev->next_rq->next_rq;
                return;
            }
        }
    }
}

/**
 * Pop the first task off the queue, NULL if no tasks.
 */
struct task_struct *rr_queue_pop(struct queue *queue) {
    if(!queue || !queue->tasks)
        return NULL;
    else {
        /* Pop off the first task */
        struct task_struct *task = queue->tasks;
        queue->tasks = queue->tasks->next_rq;
        return task;
    }
}

/**
 * Exchange the tasks of the run_queue and the just_ran_queue.
 * Called when the run_queue is depleted.
 */
void exchange_queues(void) {
    struct task_struct *tmp;
    tmp = just_ran_queue.tasks;
    just_ran_queue.tasks = run_queue.tasks;
    run_queue.tasks = tmp;
}

/**
 * Pick the highest priority task to run.
 */
struct task_struct *rr_pick_next_task(void) {
    struct task_struct *task;
    task = rr_queue_pop(&run_queue);
    if(!task) {
        /* Swap run and just_ran queues */
        exchange_queues();
        /* Try again */
        task = rr_queue_pop(&run_queue);
    }

    /* If no other tasks, but the current is still runnable, then run it! */
    if(!task && curr_task->state == TASK_RUNNABLE) {
        /* TODO: reset timeslice */
        reset_timeslice(curr_task);
        task = curr_task;
    }

    if(!task) {
        /* TODO: return idle task if no task */
        debug_queues();
        kpanic("TODO: no task to run, return idle task\n");
        return &kernel_task;
    }
    return task;
}


void debug_queues(void) {
    int i = 1;
    struct task_struct *task;
    debug("run_queue:\n");
    for(task = run_queue.tasks; task != NULL; task = task->next_rq) {
        debug("#%d: %s\n", i, task->cmdline);
        i++;
    }
    i = 1;
    debug("just_ran_queue:\n");
    for(task = just_ran_queue.tasks; task != NULL; task = task->next_rq) {
        debug("#%d: %s\n", i, task->cmdline);
        i++;
    }
    i = 1;
    debug("block_queue:\n");
    for(task = block_queue.tasks; task != NULL; task = task->next_rq) {
        debug("#%d: %s\n", i, task->cmdline);
        i++;
    }
}
