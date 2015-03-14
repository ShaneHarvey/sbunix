#include <sbunix/sbunix.h>
#include <sbunix/mm/physmem.h>

/**
* Head of LL of ppagezone{}'s
* To simplify allocation of zones on startup this is a static array for now.
*/
#define PZONE_MAX_NUM 10
static int pzone_num = 0; /* Number of pzones (and first free slot) */
static struct pzone pzones[PZONE_MAX_NUM];

/* Array of ppage{}'s, one per physical page. */
static struct ppage *ppages;


/**
* Info the OS about a range of physical memory.
* Adds a new pzone.
*
* @start:   start addr of zone.
* @end:     first addr after end of zone.
* @zflags:  zflags for new pzone.
* @return:  ptr to list of pzones or NULL.
*/
struct pzone *pzone_new(uint64_t startpage, uint64_t endpage, uint32_t zflags) {
    /* todo dynamic pzone creation */
    if(pzone_num < 0 || pzone_num >= PZONE_MAX_NUM)
        return NULL;

    startpage = ALIGN_UP(startpage, PAGE_SIZE);
    endpage = ALIGN_DOWN(endpage, PAGE_SIZE);

    if(0 == startpage) {
        startpage += 0x1000;
    }

    if(endpage <= startpage)
        return NULL;

    /* Create new zone */
    pzones[pzone_num].zflags    = zflags;
    pzones[pzone_num].start     = startpage;
    pzones[pzone_num].end       = endpage;
    pzones[pzone_num].ppages    = NULL;
    pzones[pzone_num].next      = NULL;
    if(pzone_num > 0) {
        pzones[pzone_num - 1].next = &pzones[pzone_num];
    }

    pzone_num++;
    /* Returned to ensure that pzone_new() is called prior to pmem_init()*/
    return pzones;
}

/**
* Initialize physical memory meta-data.
*
* @pzonehead:  obtained from a call to pzone_new()
*/
void pmem_init(struct pzone *pzonehead) {
    struct pzone *prev = NULL;
    struct pzone *curr = pzonehead;
    int x = 1;

    ppages = NULL;/* todo remove? */

    /* Try to allocate space for pzone->ppages in the first pages of zone. */
    for(;curr != NULL; prev = curr, curr = curr->next) {
        uint64_t npages = (curr->end - curr->start) >> PAGE_SHIFT;
        curr->ppages = NULL;
        prev->ppages = NULL; /* todo remove? */

        printf("PZone%d: [%lx-%lx] has %ld pages.\n",  x, curr->start, curr->end, npages);
        x++;
    }
}
