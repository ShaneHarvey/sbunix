#include <sbunix/sbunix.h>
#include <sbunix/mm/align.h>
#include <sbunix/mm/pt.h>

/*
 * This file sets the cpu in IA-32e Paging mode which enables 64-bit page tables
 */

void walk_pages(void) {
    uint64_t cr3;
    cr3 = read_cr3();
    printf("cr3: %p\n", cr3);
    printf("*cr3: %p\n", *(uint64_t*)cr3);
    printf("**cr3: %p\n", *(uint64_t *)(ALIGN_DOWN(*(uint64_t*)cr3, 4096)));
    printf("***cr3: %p\n", *(uint64_t *)ALIGN_DOWN(*(uint64_t *)(ALIGN_DOWN(*(uint64_t*)cr3, 4096)),4096));

}