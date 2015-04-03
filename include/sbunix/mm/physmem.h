#ifndef _SBUNIX_MM_PHYSMEM_H
#define _SBUNIX_MM_PHYSMEM_H

#include <sbunix/mm/align.h> /* alignment macros */

#define PAGE_SIZE   4096
#define PAGE_SHIFT  12

/**
* Physical page Zone, contains region specific info about the physical
* memory on the system.
*/
struct pzone {
    uint32_t zflags; /* Zone Flags, availability/type of memory range */
    uint64_t start; /* Address of first page in range (Page frame aligned). */
    uint64_t end;   /* First address after range (Page frame aligned). */
    struct ppage *ppages; /* Array of info on page frames in this range. MAY REMOVE */
    /*struct pzone *next;*/   /* Next phys_range{} */
};

enum zflags {
    PZONE_USABLE   = 0x001,
    PZONE_KERNEL   = 0x002
};

/* Calculate the number of pages in this pzone. (returns uint64_t) */
#define PZONE_NUM_PAGES(pzonep) ( ((pzonep)->end - (pzonep)->start) >> PAGE_SHIFT )


/**
* Physical Page descriptor, one per physical page frame on the system.
* Contains ptr to anon_vma{} or addr_space{}.
*/
struct ppage {
    uint32_t pflags; /* Physical page flags. */
    uint32_t mapcount; /* Count of mappings 0, 1, 2, .... */
    /*
    * If (mapping & 1) == 0: addr_space{}
    * If (mapping & 1) == 1: anon_vma{}
    */
    void *mapping;
};

enum pflags {
    PPAGE_USED     = 0x001,
    PPAGE_KERNEL   = 0x002
};


void pzone_new(uint64_t startpage, uint64_t endpage, uint32_t zflags);
void pzone_remove(uint64_t startpage, uint64_t endpage);

void physmem_init(void);
void physmem_report(void);


#endif
