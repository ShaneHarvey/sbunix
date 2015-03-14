#ifndef _SBUNIX_MM_PHYSMEM_H
#define _SBUNIX_MM_PHYSMEM_H

#define PAGE_SIZE   4096
#define PAGE_SHIFT  12

/* Used by ALIGN_DOWN */
#define ALIGN_MASK(x, mask)     ( ((x) + (mask)) & ~(mask) )

/* Rounds down to multiple of almt (alignment), must power of 2 */
#define ALIGN_DOWN(ptr, almt)   ( (ptr) & ~(((__typeof__(ptr))(almt) - 1)) )

/* Same as above but rounds up. */
#define ALIGN_UP(ptr, almt)   ALIGN_MASK(ptr, ( ((__typeof__(ptr))(almt) - 1) ))

/**
* Physical page Zone, contains region specific info about the physical
* memory on the system.
*/
struct pzone {
    uint32_t zflags; /* Zone Flags, availability/type of memory range */
    uint64_t start; /* Address of first page in range (Page frame aligned). */
    uint64_t end;   /* First address after range (Page frame aligned). */
    struct ppage *ppages; /* Array of info on page frames in this range. MAY REMOVE */
    struct pzone *next;   /* Next phys_range{} */
};

enum zflags {
    PZONE_USABLE   = 0x001,
    PZONE_KERNEL   = 0x002
};

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
    PPAGE_AVAIL    = 0x002,
    PPAGE_KERNEL   = 0x004
};


struct pzone *pzone_new(uint64_t startpage, uint64_t endpage, uint32_t zflags);

void pmem_init(struct pzone *pzonehead);


#endif
