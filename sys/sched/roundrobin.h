#ifndef SCHED_ROUNDROBIN_H
#define SCHED_ROUNDROBIN_H

#include <sbunix/sched.h>

extern struct queue just_ran_queue;

void rr_queue_add(struct queue *queue, struct task_struct *task);
struct task_struct *rr_queue_pop(struct queue *queue);
void rr_queue_remove(struct queue *queue, struct task_struct *task);
void exchange_queues(void);
struct task_struct *rr_pick_next_task(void);

void debug_queues(void);

#endif
