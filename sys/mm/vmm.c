#include <sbunix/mm/vmm.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>

/*
 * Virtual Memory for user processes, should be mostly operations on
 * vm_area's and mm_struct's.
 *
 * In page fault handler:
 *      1. Use curr_task to retrieve mm_struct
 *      2. Call vma_find(mm->vmas, fault_addr)
 *      3. Call vma->onfault(vma, fault_addr)
 *          * onfault will take appropriate action (map_page(), kill user, etc...)
 */


/* Private functions */
int vma_overlaps(struct vm_area *vma, struct vm_area *other);
int vma_contains(struct vm_area *vma, uint64_t addr);
void mm_list_add(struct mm_struct *mm);

/**
 * Create an mm_struct
 */
struct mm_struct *mm_create(uint64_t start_code,
                            uint64_t end_code,
                            uint64_t start_data,
                            uint64_t end_data) {
    struct mm_struct *mm;

    mm = kmalloc(sizeof(*mm));
    if(!mm)
        return NULL;

    mm->vmas = NULL;
    mm->pgd = NULL; /* todo: when do we copy pt's ??? */
    mm->mm_count = 1;
    mm->vma_count = 0;
    mm_list_add(mm);
    mm->start_code = start_code;
    mm->end_code = end_code;
    mm->start_data = start_data;
    mm->end_data = end_data;

    mm->brk = mm->start_brk = ALIGN_UP(end_data, PAGE_SIZE);
    mm->start_stack = USER_START_STACK;
    return mm;
}


void mm_destroy(struct mm_struct *mm) {
    if(!mm)
        return;

    if(--mm->mm_count <= 0) {
        /* fixme: free all vma's, free page tables */
        if(mm->mm_next) {
            mm->mm_next->mm_prev = mm->mm_prev;
        }
        if(mm->mm_prev) {
            mm->mm_prev->mm_next = mm->mm_next;
        }
        /* fixme: add other kfree()'s */
        kfree(mm);
    }
}

/**
 * Add the mm to the list of ALL mm_structs in the system.
 */
void mm_list_add(struct mm_struct *mm) {
    if(!mm)
        return;

    if(kernel_mm.mm_prev && kernel_mm.mm_next) {
        /* Insert mm behind the global kernel_mm */
        kernel_mm.mm_prev->mm_next = mm;
        mm->mm_prev = kernel_mm.mm_prev;
        kernel_mm.mm_prev = mm;
        mm->mm_next = &kernel_mm;
    } else {
        /* kernel has no next or prev */
        kernel_mm.mm_next = mm;
        kernel_mm.mm_prev = mm;
        mm->mm_next = &kernel_mm;
        mm->mm_prev = &kernel_mm;
    }
}

/**
 * Add a vma into the vma list of mm.
 * NOTE: Must add lowest address area first (text vm_area)
 */
int mm_add_vma(struct mm_struct *mm, struct vm_area *vma) {
    struct vm_area *curr;
    if(!mm || !vma || vma->vm_start >= vma->vm_end)
        return -1;

    if(!mm->vmas) {
        mm->vmas = vma;
        mm->vma_count++;
        return 0;
    }
    curr = mm->vmas;
    for(; curr != NULL; curr = curr->vm_next) {
        /* overlapping is bad */
        if(vma_overlaps(vma, curr))
            return -1;

        /* After curr? */
        if (curr->vm_end <= vma->vm_start) {
            /* Before next? */
            if(!curr->vm_next || vma->vm_end <= curr->vm_next->vm_start) {
                /* insert here! */
                vma->vm_next = curr->vm_next;
                curr->vm_next = vma;
                return 0;
            }
        }
    }
    return -1;
}

/******************/
/* VMA operations */
/******************/

/**
 * Allocate and fill a vm_area.
 */
struct vm_area *vma_create(uint64_t vm_start, uint64_t vm_end, ulong vm_prot) {
    struct vm_area *vma;
    vma = kmalloc(sizeof(*vma));
    if(!vma)
        return NULL;
    memset(vma, 0, sizeof(*vma));
    vma->vm_start = vm_start;
    vma->vm_end = vm_end;
    vma->vm_prot = vm_prot;
    return vma;
}

/**
 * Free a vm_area.
 */
void vma_destroy(struct vm_area *vma) {
    /* todo: free all pages in vm_area */
    kfree(vma);
}

/**
 * Return 1 if vma contains the address addr, 0 otherwise.
 */
int vma_contains(struct vm_area *vma, uint64_t addr) {
    if(!vma)
        return 0;
    return (vma->vm_start <= addr && addr < vma->vm_end);
}

/**
 * Find a vma containing the user virtual address addr.
 * Return NULL if not found.
 */
struct vm_area *vma_find(struct vm_area *vma, uint64_t addr) {
    for(; vma != NULL; vma = vma->vm_next)
        if(vma_contains(vma, addr))
            break;
    return vma;
}

/**
 * Return 1 if overlapping, 0 if not
 */
int vma_overlaps(struct vm_area *vma, struct vm_area *other) {
    /* testing if vma intersects with other */
    int whole, middle, clip_left, clip_right;
    whole = vma->vm_start <= other->vm_start
            && other->vm_end <= vma->vm_end;

    middle = other->vm_start <= vma->vm_start
             && vma->vm_end <= other->vm_end;

    clip_left = vma->vm_start <= other->vm_start
                && other->vm_start < vma->vm_start;

    clip_right = other->vm_start < vma->vm_start
                 && vma->vm_end < other->vm_end;
    return whole | middle | clip_left | clip_right;
}

/**
 * Test vm_area.onfault function.
 * A real onfault will take appropriate actions (map_page(), kill user, etc...)
 */
int test_onfault(struct vm_area *vma, uint64_t addr) {
    char *msg[] = {
        "is not in",
        "is in"
    };
    debug("0x%lx %s the vm_area\n", addr, msg[vma_contains(vma, addr)]);
    //map_page(....)
    return 0;
}

