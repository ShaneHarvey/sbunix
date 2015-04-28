#ifndef _SBUNIX_PAGE_ALLOC_H
#define _SBUNIX_PAGE_ALLOC_H

#include <sbunix/mm/physmem.h>

/* When a page is free it is an element on the freepage linked list. */
struct freepage  {
    struct freepage *next;
    char _pad[PAGE_SIZE - sizeof(struct freepage*)];
};

struct freepagehd {
    uint64_t nfree;
    uint64_t maxfree;
    struct freepage *freepages;
};

extern struct freepagehd freepagehd;

uint64_t get_free_page(uint32_t gpf_flags);
uint64_t get_phys_page(void);
uint64_t get_zero_page(void);
void free_page(uint64_t virt_page_addr);
void freemem_report(void);

#endif
