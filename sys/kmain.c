#include "kmain.h"
#include <sbunix/sbunix.h>
#include <sbunix/sched.h>
#include <sbunix/mm/pt.h>
#include <sbunix/writec.h>
#include <sbunix/interrupt/pit.h>
#include <sbunix/time.h>
#include <sbunix/tarfs.h>

void printA(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    while(1) {
        printk("%d %d %d %d %d %d %d %d\n", a, b, c, d, e ,f, g, h);
        schedule();
        a++;b++;c++;d++;e++;f++;g++;h++;
    }
}

void printB(void) {
    int a = -1, b = -2, c = -3, d = -4, e = -5, f = -6, g = -7, h = -8;
    while(1) {
        printk("%d %d %d %d %d %d %d %d\n", a, b, c, d, e ,f, g, h);
        schedule();
        a--;b--;c--;d--;e--;f--;g--;h--;
    }
}

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    int i;
    clear_console();
    printk("*** Welcome to SBUnix ***\n");
    printk("Starting task test...\n");
    ktask_create(printA);
    ktask_create(printB);
    printk("Created the threads\n");

    for(i = 0; i < 5; i++) {
        printk("Main Task\n");
        schedule();
    }
    test_read_tarfs();
    kpanic("\nReturned to kmain!!!\n");
}
