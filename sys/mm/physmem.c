#include <sbunix/sbunix.h>
#include <sbunix/string.h>
#include <sbunix/mm/physmem.h>
#include <sbunix/mm/page_alloc.h>


/**
* Deals with bootstrapping physical memory management.
*
* The pzones have a static array which is returned by pzone_new().
* After calls to pzone_new(), call pzone_remove() with regions that must
* be reserved (currently for the kernel's memory, physbase to physfree).
* Finally call physmem_init()
*/


/**
* Array of pzone{}'s
* To simplify allocation of zones on startup this is a static array for now.
*/
#define PZONE_MAX_NUM 10
static size_t pzone_num = 0; /* Number of pzones (and first free slot) */
static struct pzone pzones[PZONE_MAX_NUM];

/* Private functions. */
static void _ppage_new(struct ppage *base, size_t nppages);
static void _pzone_entry(size_t i, uint64_t startpage, uint64_t endpage, uint32_t zflags);
void _pzone_ppages_init(struct pzone *pzone);
void _create_free_page_list(struct pzone *base);


/**
* Create an entry. Caller MUST align startpage/endpage to PAGE_SIZE.
*/
void _pzone_entry(size_t i, uint64_t startpage, uint64_t endpage, uint32_t zflags) {
    pzones[i].zflags    = zflags;
    pzones[i].start     = startpage;
    pzones[i].end       = endpage;
    pzones[i].ppages    = NULL;
}

/**
* Inform the OS about a range of physical memory.
* Calls to pzone_new() MUST be in increasing order of zones and
* MUST not overlap.
*
* @startpage:   start addr of zone.
* @endpage:     first addr after end of zone.
* @zflags:      zflags for new pzone.
* @return:      ptr to list of pzones or NULL.
*/
struct pzone *pzone_new(uint64_t startpage, uint64_t endpage, uint32_t zflags) {
    /* todo dynamic pzone creation */
    if(pzone_num >= PZONE_MAX_NUM)
        return NULL;

    startpage = ALIGN_UP(startpage, PAGE_SIZE);
    endpage = ALIGN_DOWN(endpage, PAGE_SIZE);

    if(0 == startpage) { /* Disallow page 0 access */
        startpage += 0x1000;
    }

    if(endpage <= startpage)
        return NULL;

    /* Create new zone */
    _pzone_entry(pzone_num, startpage, endpage, zflags);

    pzone_num++;
    /* Returned to ensure that pzone_new() is called prior to physmem_init()*/
    return pzones;
}

/**
* Use to remove or split a pzone (e.g. kernel's memory region).
* If this region is currently a pzone, remove or split it up.
*
* @startpage:   start addr of zone.
* @endpage:     first addr after end of zone.
* @return:      ptr to list of pzones or NULL.
*/
struct pzone *pzone_remove(uint64_t startpage, uint64_t endpage) {
    size_t i = 0;

    startpage = ALIGN_DOWN(startpage, PAGE_SIZE);
    endpage = ALIGN_UP(endpage, PAGE_SIZE);

    if(endpage <= startpage)
        return NULL;

    while(i < pzone_num) {
        /* For clarity there are 4 cases for the way the remove region can
        * overlap with the pzone's region.
        * Assumes the global pzones may be unordered */
        if(startpage <= pzones[i].start && pzones[i].end <= endpage) {
            /* rm whole region */
            /* Delete here, then move the rest of the array back 1 slot */
            memset((pzones+i), 0, sizeof(struct pzone));
            if(pzone_num > i + 1)
                memmove((pzones+i), (pzones+i+1), sizeof(struct pzone) * (pzone_num-i-1));
            pzone_num--;
            continue; /* don't inc i */

        } else if(startpage <= pzones[i].start && endpage <= pzones[i].end) {
            /* rm beginning of pzone. */
            pzones[i].start = endpage;

        } else if(startpage < pzones[i].end && pzones[i].end <= endpage) {
            /* rm end of pzone. */
            pzones[i].end = startpage;

        } else if(pzones[i].start < startpage && endpage < pzones[i].end) {
            /* rm middle of pzone. */
            uint64_t oldend = pzones[i].end;
            pzones[i].end = startpage;
            /* Move the rest of the array forward 1 slot */
            if(pzone_num >= PZONE_MAX_NUM)
                return NULL; /* We could recover here, instead of failing */
            if(pzone_num > i + 1)
                memmove((pzones+i+1), (pzones+i+2), sizeof(struct pzone) * (pzone_num-i-1));

            _pzone_entry(i+1, endpage, oldend, pzones[i].zflags);
            pzone_num++;
            i++; /* i should be inc'd twice */
        }

        i++;
    }

    if(pzone_num == 0)
        return NULL;
    return pzones;
}


/**
* Fill array of ppage{}s
*
* @base:    start address of ppage array.
* @nppages: number of ppages in array base[].
*/
void _ppage_new(struct ppage *base, size_t nppages) {
    memset(base, 0, nppages);
}

/**
* Try to allocate space for pzone->ppages in the first pages of zone.
*/
void _pzone_ppages_init(struct pzone *pzone) {
    uint64_t npages = PZONE_NUM_PAGES(pzone);
    uint64_t needbytes = npages * sizeof(struct ppage);
    uint64_t needpages = ALIGN_UP(needbytes, PAGE_SIZE) >> PAGE_SHIFT;

    /*debug("PZ%ld: [%lx-%lx] has %ld pages.\n", i, base[i].start, base[i].end, npages);
    debug("PZ%ld: needbytes %ld, needpages %ld.\n", i, needbytes, needpages);*/
    if(needpages >= npages)
        return;

    /* Put ppage array in first pages of the pzone */
    pzone->ppages = (struct ppage*)pzone->start;
    _ppage_new(pzone->ppages, needpages);

    /* Bump up the start address, possibly wasting space. */
    pzone->start = ALIGN_UP(pzone->start + needbytes, PAGE_SIZE);
}


/**
* Initialize physical memory meta-data.
*
* @pzonehead:  obtained from a call to pzone_new()
*/
void physmem_init(struct pzone *base) {
    size_t i = 0;
    if(!base)
        kpanic("No physical zones?!?!\n");

    for(;i < pzone_num; i++) {
        _pzone_ppages_init(base + i);
        debug("pz%ld: [%lx-%lx] has %ld pages.\n", i, base[i].start, base[i].end, PZONE_NUM_PAGES(base + i));
    }

    _create_free_page_list(base);
}

/**
* Print known information on physical memory.
*/
void physmem_report(void) {
    size_t i = 0;
    uint64_t totpages = 0, npages;

    for(; i < pzone_num; i++) {
        npages = PZONE_NUM_PAGES(pzones + i);
        totpages += npages;
    }
    debug("%ld total ppages across %ld pzones.\n", totpages, pzone_num);
}



/**
* Create free list of pages.
*
* @base: Array of pzone{}'s
*/
void _create_free_page_list(struct pzone *base) {
    size_t i;
    struct freepage *prev = NULL, *curr =NULL;

    if(!base)
        kpanic("No physical zones?!?!\n");

    /* First we find assign the free page head. */
    for(i = 0; i < pzone_num; i++) {
        if(!(base[i].zflags & PZONE_USABLE))
            continue;

        if(PZONE_NUM_PAGES(base + i) <= 0)
            continue;

        freepagehd.freepages = (struct freepage *) base[i].start;
        prev = freepagehd.freepages;
        debug("Found freepagehd at %p\n", freepagehd.freepages);
        break;
    }

    if(!freepagehd.freepages)
        kpanic("No usable pages?!?!\n");

    /* Now we build the free list. */
    for(; i < pzone_num; i++) {
        if(!(base[i].zflags & PZONE_USABLE))
            continue;

        /* todo remove. Test page faults */
        //((struct freepage*)0x40000)->next = (struct freepage*)0x41000;
        //debug("test: %lx-> = %lx\n", 0x40000, 0x41000);

        curr = (struct freepage*) base[i].start;
        debug("pz%ld adding [%lx-%lx] to freelist.\n", i, base[i].start, base[i].end);
        for(;curr < (struct freepage*) base[i].end; prev = curr, curr++) {
            //debug("%p->next == %p\n", prev, curr);
            prev->next = curr;
            freepagehd.nfree++;
        }
    }
    prev->next = NULL;

    debug("%ld free pages.\n", freepagehd.nfree);
}