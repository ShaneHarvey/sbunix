#include "kmain.h"
#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/mm/pt.h>


struct mm_struct kernel_mm = {0};

struct task_struct init_task = {
        .state = TASK_RUNNABLE,
        .kernel_rsp = 0, /* Will be set on first call to schedule */
        .mm = NULL,
        .active_mm = &kernel_mm,
        .next_task = NULL,
        .prev_task = NULL
};

struct task_struct *create_kthread(void(*start)(void)) {
    struct task_struct *task;
    uint64_t *stack;

    stack = (uint64_t *)get_free_page(0);
    if(!stack)
        return NULL;
    task = kmalloc(sizeof(struct task_struct));
    if(!task) {
        free_page((uint64_t)stack);
        return NULL;
    }
    stack[511] = (uint64_t)start;
//  stack[510] = (uint64_t)r15;
//  stack[509] = (uint64_t)r14;
//  stack[508] = (uint64_t)r13;
//  stack[507] = (uint64_t)r12;
//  stack[506] = (uint64_t)rbp;
//  stack[505] = (uint64_t)rbx;
//  stack[504] = (uint64_t)unused;
    stack[503] = (uint64_t)task;
    task->kernel_rsp = (uint64_t)&stack[503];

    task->active_mm = &kernel_mm;
    task->mm = NULL;

    return task;
}

void printA(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    while(1) {
        printf("%d %d %d %d %d %d %d %d\n", a, b, c, d, e ,f, g, h);
        schedule();
        a++;b++;c++;d++;e++;f++;g++;h++;
    }
}

void printB(void) {
    while(1) {
        printf("B\n");
        schedule();
    }
}

void printC(void) {
    int a = -1, b = -2, c = -3, d = -4, e = -5, f = -6, g = -7, h = -8;
    while(1) {
        printf("%d %d %d %d %d %d %d %d\n", a, b, c, d, e ,f, g, h);
        schedule();
        a--;b--;c--;d--;e--;f--;g--;h--;
    }
}

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    struct task_struct *a, *b, *c;
    int i;

    a = create_kthread(printA);
    b = create_kthread(printB);
    c = create_kthread(printC);

    init_task.next_task = a;
    init_task.prev_task = c;
    a->prev_task = &init_task;
    a->next_task = b;
    b->next_task = c;
    b->prev_task = a;
    c->next_task = &init_task;
    c->prev_task = b;

    run_queue.tasks = a;
    run_queue.curr = &init_task;
    for(i = 0; i < 10; i++) {
        printf("Main Task\n");
        schedule();
    }
    kpanic("\nReturned to kmain!!!\n");
}
