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
    struct freepage *freepages;
};

extern struct freepagehd freepagehd;

#endif
