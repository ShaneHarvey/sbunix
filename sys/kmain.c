#include "kmain.h"
#include <sbunix/sbunix.h>
#include <sbunix/mm/physmem.h>
#include <sbunix/mm/pt.h>

/**
 * The entry point of our kernel. Here we have access to a page allocator and
 * interrupts are enabled.
 */
void kmain(void) {
    /* Init kernel page table */
    init_kernel_pt();
}
