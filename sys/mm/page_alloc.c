#include <sbunix/sbunix.h>
#include <sbunix/mm/page_alloc.h>

/* This file deals with allocating physical pages. */

/* Private functions. */
ulong _free_page_pop(void);

/* Global head of the free page linked list. */
struct freepagehd freepagehd = { .nfree = 0, .freepages = NULL};

/**
* Return the logical address of a usable page of memory.
* todo: actually use gpf_flags
*/
uint64_t get_free_page(uint32_t gpf_flags) {
    uint64_t logical_addr;
    if(freepagehd.nfree == 0)
        return 0;

    logical_addr = _free_page_pop();

    return logical_addr;
}

void free_page(uint64_t virt_page_addr) {

}

/**
* Pop the head off the free page list, and return it.
* Also mark the corresponding ppage{} appropiately.
* Returns a logical address.
*/
uint64_t _free_page_pop(void) {
    struct freepage* retval;

    if(freepagehd.nfree == 0)
        return 0;

    retval = freepagehd.freepages; /* grab head free page */
    if(!retval)
        kpanic("freepages=NULL, but nfree=%lu!\n", freepagehd.nfree);

    freepagehd.freepages = retval->next;
    /* todo: maybe need to convert a virt to phys (once kern page tables set up) */

    //ppage_mark_used();

    return (uint64_t)retval;
}
