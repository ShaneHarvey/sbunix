#include "kmain.h"
#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/mm/pt.h>

void printA(void) {
    while(1) {
        printf("A");
        schedule();
    }
}

void printB(void) {
    while(1) {
        printf("B");
        schedule();
    }
}

void printC(void) {
    while(1) {
        printf("C");
        schedule();
    }
}

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    struct task_struct *a, *b, *c;
    int i;

    printf("Starting task test...\n");
    a = ktask_create(printA);
    task_add_new(a);
    b = ktask_create(printB);
    task_add_new(b);
    c = ktask_create(printC);
    task_add_new(c);
    printf("Created the threads\n");

    for(i = 0; i < 5; i++) {
        printf("\nschedule");
        schedule();
    }
    kpanic("\nReturned to kmain!!!\n");
}
