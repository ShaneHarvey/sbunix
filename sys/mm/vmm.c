#include <sbunix/mm/vmm.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>

/*
 * Virtual Memory for user processes, should be mostly operations on
 * vm_area's and mm_struct's.
 *
 * In page fault handler:
 *      1. Use curr_task to retrieve mm_struct
 *      2. Call vma_find_region(mm->vmas, fault_addr, 0)
 *      3. Call vma->onfault(vma, fault_addr)
 *          * onfault will take appropriate action (map_page(), kill user, etc...)
 *
 * In syscalls:
 *      1. Call validate_userptr()
 */


/* Private functions */
int mm_add_vma(struct mm_struct *mm, struct vm_area *vma);
void mm_list_add(struct mm_struct *mm);
int vma_overlaps(struct vm_area *vma, struct vm_area *other);
int vma_contains(struct vm_area *vma, uint64_t addr);
int vma_contains_region(struct vm_area *vma, uint64_t addr, size_t size);
int vma_grow_up(struct vm_area *vma, uint64_t new_end);


/**
 * Create an mm_struct for a user.
 */
struct mm_struct *mm_create_user(uint64_t start_code, uint64_t end_code,
                                 uint64_t start_rodata, uint64_t end_rodata,
                                 uint64_t start_data, uint64_t end_data) {
    struct mm_struct *mm;
    struct vm_area *vma;
    mm = mm_create();
    if(!mm)
        return NULL;
    mm->start_code = start_code;
    mm->end_code = end_code;
    mm->start_rodata = start_rodata;
    mm->end_rodata = end_rodata;
    mm->start_data = start_data;
    mm->end_data = end_data;
    mm->brk = mm->start_brk = ALIGN_UP(end_data, PAGE_SIZE);
    mm->start_stack = USER_STACK_START;

    /* Code vma */
    vma = vma_create(start_code, end_code, VM_CODE, VM_READ | VM_EXEC);
    if(!vma)
        goto out_vma;
    /* fixme: file? when is code mapped? */
    mm_add_vma(mm, vma);

    vma = vma_create(start_rodata, end_rodata, VM_RODATA, VM_READ);
    if(!vma)
        goto out_vma;
    /* fixme: when is rodata mapped? */
    mm_add_vma(mm, vma);

    vma = vma_create(start_data, end_data, VM_DATA, VM_READ | VM_WRITE);
    if(!vma)
        goto out_vma;
    /* fixme: when is data mapped? */
    mm_add_vma(mm, vma);

    vma = vma_create(mm->start_brk, mm->brk, VM_HEAP, VM_READ | VM_WRITE);
    if(!vma)
        goto out_vma;
    /* fixme: when is heap mapped? */
    mm_add_vma(mm, vma);

    vma = vma_create(USER_STACK_END, mm->start_stack, VM_STACK, VM_READ | VM_WRITE);
    if(!vma)
        goto out_vma;
    /* fixme: map the stack! */
    mm_add_vma(mm, vma);

out_vma:
    mm_destroy(mm);  /* also destroys all vma's */
    return NULL;
}

/**
 * Create an mm_struct.
 */
struct mm_struct *mm_create(void) {
    struct mm_struct *mm;
    mm = kmalloc(sizeof(*mm));
    if(!mm)
        return NULL;

    memset(mm, 0, sizeof(*mm));
    /* todo: when do we copy pt's ??? */
    mm->mm_count = 1;
    mm_list_add(mm);
    return mm;
}

/**
 * Decrement the reference count of the mm, if mm_count goes to 0:
 *      * Free all the vma's and then mm struct itself.
 * Does NOT destroy the page tables!!
 */
void mm_destroy(struct mm_struct *mm) {
    if(!mm)
        return;

    if(--mm->mm_count <= 0) {
        vma_destroy_all(mm);

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
 * Real work for srbk()
 *    new break     -- on success
 *    current break -- on failure
 */
uint64_t mm_do_sys_sbrk(struct mm_struct *mm, uint64_t newbrk) {
    if(!mm)
        return (uint64_t)-1;
    if(newbrk <= mm->brk)
        return mm->brk;
    else {
        struct vm_area *vma = vma_find_type(mm->vmas, VM_HEAP);
        if(!vma)
            kpanic("No heap vm area found!\n");
        if(-1 == vma_grow_up(vma, ALIGN_UP(newbrk, PAGE_SIZE)))
            return mm->brk;
        else
            return newbrk;
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
 * fixme: Must add lowest address area first (text vm_area)
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

/**
 * Checks if the user pointer is within a valid virtual memory area.
 * If it is, map the corresponding page if it is not present in the
 * page table.
 * @return:  0 if userp is valid and is safe to use (mapped), or
 *          -1 if not valid,
 *          -2 if mapping failed (example: no memory)
 */
int validate_userptr(struct mm_struct *mm, userptr_t userp, size_t size) {
    struct vm_area *vma;
    int present = 0;

    vma = vma_find_region(mm->vmas, (uint64_t)userp, size);
    if(!vma)
        return -1;  /* not valid */

    /* fixme!!!! magic test if page present */
    if(present) {
        return 0; /* valid and present, won't page fault */
    }

    return -1;
}


/******************/
/* VMA operations */
/******************/

/**
 * Allocate and fill a vm_area.
 */
struct vm_area *vma_create(uint64_t vm_start, uint64_t vm_end,
                           vm_type_t type, ulong vm_prot) {
    struct vm_area *vma;
    vma = kmalloc(sizeof(*vma));
    if(!vma)
        return NULL;
    memset(vma, 0, sizeof(*vma));
    vma->vm_type = type;
    vma->vm_start = vm_start;
    vma->vm_end = vm_end;
    vma->vm_prot = vm_prot;
    return vma;
}

/**
 * Free a vm_area.
 */
void vma_destroy(struct vm_area *vma) {
    if(!vma)
        return;

    if(vma->vm_file) {
        /* fixme: JUST CALL free_pagetbl_range */
    } else {
        /* fixme: free all pages in vm_area. CALL free_pagetbl_range_and_pages */
    }
    kfree(vma);
}

/**
 * Free all the vma's in a mm_struct
 */
void vma_destroy_all(struct mm_struct *mm) {
    struct vm_area *prev, *next;
    if(!mm)
        return;

    for(prev = mm->vmas; prev != NULL; prev = next) {
        next = prev->vm_next;
        vma_destroy(prev);
    }
}

/**
 * @return: 1 if vma contains the address addr,
 *          0 otherwise.
 */
int vma_contains(struct vm_area *vma, uint64_t addr) {
    if(!vma)
        return 0;
    return (vma->vm_start <= addr && addr < vma->vm_end);
}

/**
 * @return: 1 if vma contains the full region [addr, addr+size)
 *          0 otherwise
 */
int vma_contains_region(struct vm_area *vma, uint64_t addr, size_t size) {
    if(!vma)
        return 0;
    return (vma->vm_start <= addr && (addr+size) < vma->vm_end);
}

/**
 * Find a vma containing the user virtual address addr.
 * @return: NULL if not found.
 */
struct vm_area *vma_find_region(struct vm_area *vma, uint64_t addr, size_t size) {
    for(; vma != NULL; vma = vma->vm_next)
        if(vma_contains_region(vma, addr, size))
            break;
    return vma;
}

/**
 * Find the first vma of type type
 * @return: NULL if not found.
 */
struct vm_area *vma_find_type(struct vm_area *vma, vm_type_t type) {
    for(; vma != NULL; vma = vma->vm_next)
        if(vma->vm_type == type)
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
        "NOT",
        ""
    };
    debug("0x%lx is %s in the vm_area\n", addr, msg[vma_contains(vma, addr)]);
    //map_page(....)
    return 0;
}


/**
 * Set the vma's vm_end to the page aligned new_end.
 * @new_end: MUST be page aligned!!
 * @return: 0 if extended
 *          -1 if new_end is less than the current or it hits the next vm area
 */
int vma_grow_up(struct vm_area *vma, uint64_t new_end) {
    if(!vma)
        return -1;
    if(new_end < vma->vm_end)
        return -1;
    if(vma->vm_next && new_end <= vma->vm_next->vm_start) {
        vma->vm_end = new_end;
        return 0;
    }
    return -1;  /* new_end would overlap next vm area */
}