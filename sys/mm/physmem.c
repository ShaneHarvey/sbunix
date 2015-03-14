#include <sbunix/sbunix.h>
#include <sys/defs.h>

/* Rounds down to multiple of almt. */
#define ALIGN_TO(ptr, almt) ((ptr) & ~((typeof(ptr))(almt) - 1))

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

/**
* Head of LL of ppagezone{}'s
* To simplify allocation of zones on startup this is a static array for now.
*/
#define PZONE_NUM 10
static int pzone_max = 0; /* Number of pzones (and first free slot) */
static struct pzone pzones[PZONE_NUM];

/* Array of ppage{}'s, one per physical page. */
static struct ppage *ppages;


/**
* Add a new pzone.
*
* @start:   start addr of zone.
* @end:     first addr after end of zone.
* @zflags:  zflags for new pzone.
* @return:  ptr to list of pzones or NULL.
*/
struct pzone *new_pzone(uint64_t start, uint64_t end, uint32_t zflags) {
    /* todo dynamic pzone creation */
    if(pzone_num < 0 || pzone_num >= PZONE_NUM)
        return NULL;


    start = ALIGN_TO(start, 0x1000);
    end = ALIGN_TO(end, 0x1000);

    if(0 == start) {
        start += 0x1000;
    }

    if(end <= start)
        return NULL;

    /* Create new zone */
    pzones[pzone_num].zflags    = zflags;
    pzones[pzone_num].start     = start;
    pzones[pzone_num].end       = end;
    pzones[pzone_num].ppages    = NULL;
    pzones[pzone_num].next      = NULL;
    if(pzone_num > 0) {
        pzones[pzone_num - 1].next = pzones[pzone_num];
    }
    pzone_num++;

    /* Returned to ensure that new_pzone() is called prior to init_pmem()*/
    return pzones;
}

/**
* Initialize physical memory meta-data.
*
* @pzonehead:  obtained from a call to new_pzone()
*/
void init_pmem(struct pzone *pzonehead) {
    struct pzone *prev = NULL;
    struct pzone *curr = pzonehead;

    /* Try to allocate space for pzone.ppages in the first pages. */
    for(;curr != NULL; prev = curr, curr = curr->next) {

    }

}
