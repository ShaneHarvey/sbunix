#include <sbunix/sbunix.h>
#include <sbunix/mm/page_alloc.h>
#include <sbunix/string.h>

/* This file deals with allocating physical pages. */

/* Global head of the free page linked list. */
struct freepagehd freepagehd = { .nfree = 0, .maxfree = 0, .freepages = NULL};

/* Private functions. */
uint64_t freepagehd_pop(void);
void freepagehd_push(struct freepage* newhd);

/**
 * Return the kernel virtual address address of a usable page of memory.
 * todo: actually use gpf_flags
 */
uint64_t get_free_page(uint32_t gpf_flags) {
    uint64_t pgaddr;
    if(freepagehd.nfree == 0)
        return 0;

    pgaddr = freepagehd_pop();
    /* TODO: remove memset to 0 from get_free_page to speed up kernel allocs */
    memset((void*) pgaddr, 0, PAGE_SIZE);
    return pgaddr;
}

/**
 * Calls get_free_page()
 * Return a physical address of a usable, zeroed page of memory.
 */
uint64_t get_zero_page(void) {
    uint64_t pgaddr;
    pgaddr = get_free_page(0);
    if(!pgaddr)
        return 0;

    memset((void*) pgaddr, 0, PAGE_SIZE);
    return kvirt_to_phys(pgaddr);
}

/**
 * Calls get_free_page()
 * Return a physical address.
 */
uint64_t get_phys_page(void) {
    uint64_t pgaddr = get_free_page(0);
    if(!pgaddr)
        return 0;
    return kvirt_to_phys(pgaddr);
}

/**
 * Free the page.
 * @virt_page_addr: kernel virtual page address
 */
void free_page(uint64_t virt_page_addr) {
    freepagehd_push((struct freepage*)virt_page_addr);
}

/**
 * Pop the head off the free page list, and return it.
 * Also mark the corresponding ppage{} appropriately.
 * Returns a physical (logical) address.
 * TODO: To add swapping-to-disk update the ppage structs
 */
uint64_t freepagehd_pop(void) {
    struct freepage* freepg;

    if(freepagehd.nfree == 0)
        return 0;

    freepg = freepagehd.freepages; /* grab head free page */
    if(!freepg)
        kpanic("Page list corrupted! freepages=NULL, but nfree=%lu!\n", freepagehd.nfree);

    freepagehd.freepages = freepg->next;
    /* todo: maybe need to convert a virt to phys (once kern page tables set up) */

    /*if(ppage_mark_used((uint64_t)freepg))
        kpanic("Unable to mark %p's ppage{} as USED!\n", freepg);*/

    freepagehd.nfree--;
    return (uint64_t) freepg;
}

/**
 * Push a new freepage{} to the front of the free page list.
 */
void freepagehd_push(struct freepage* newhd) {
    newhd->next = freepagehd.freepages;
    freepagehd.freepages = newhd;
    freepagehd.nfree++;
}

void freemem_report(void) {
    ulong percent = freepagehd.nfree * 100 / freepagehd.maxfree;
    printk("Free pages: %lu/%lu  ~%lu%%\n", freepagehd.nfree, freepagehd.maxfree, percent);
}
