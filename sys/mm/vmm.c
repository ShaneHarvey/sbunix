#include <sbunix/mm/vmm.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>
#include <sbunix/vfs/vfs.h>
#include <errno.h>

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
int vma_intersects(struct vm_area *vma, struct vm_area *other);
int vma_contains(struct vm_area *vma, uint64_t addr);
int vma_contains_region(struct vm_area *vma, uint64_t addr, size_t size);
int vma_grow_up(struct vm_area *vma, uint64_t new_end);


/**
 * Create an mm_struct for a user.
 */
struct mm_struct *mm_create_user(uint64_t start_code, uint64_t end_code,
                                 uint64_t start_data, uint64_t end_data) {
    struct mm_struct *mm;
    struct vm_area *vma;
    mm = mm_create();
    if(!mm)
        return NULL;
    mm->brk = mm->start_brk = ALIGN_UP(end_data, PAGE_SIZE);
    mm->start_stack = USER_STACK_START;

    /* Code vma */
    vma = vma_create(start_code, end_code, VM_CODE, VM_READ | VM_EXEC);
    if(!vma)
        goto out_vma;
    /* fixme: file? when is code mapped? */
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
 * Adds a heap vm area above the last currently in user_mm's vm areas.
 * Called after loading loadable segments from the ELF file.
 * @return: 1 on success, 0 on error
 */
int add_heap(struct mm_struct *user) {
    struct vm_area *vma, *heap;
    if(!user || !user->vmas)
        return 0;
    for(vma = user->vmas; vma->vm_next != NULL; vma = vma->vm_next) {
    }
    /* Heap starts after the other sections */
    user->start_brk = user->brk = ALIGN_UP(vma->vm_end + 1, PAGE_SIZE);
    /* Initially size is 0 */
    heap = vma_create(user->start_brk, user->brk, VM_HEAP, VM_READ | VM_WRITE);
    vma->onfault = onfault_mmap_anon;
    /* fixme: finish */
}

/**
 * Add a new vma into the mm struct. It is either a mmapped area
 * with a file backing, or an anon mmapped region.
 * @filep: NULL in anon mmap
 * @fstart: starting offset in the file, ignored if filep == NULL
 * @fsize: portion of the file being mapped
 * @prot: e.g. VM_READ, VM_EXEC, ...
 * @vm_start: start of vma
 * @vm_end: end of vma
 *
 * @return: 0 on error, 1 on success
 */
int mmap_area(struct mm_struct *mm, struct file *filep,
              off_t fstart, size_t fsize, ulong prot,
              uint64_t vm_start, uint64_t vm_end) {

    struct vm_area *vma;
    if(!mm)
        return -EINVAL;

    vma = vma_create(vm_start, vm_end, VM_MMAP, prot);
    if(!vma)
        return -ENOMEM;
    if(!mm_add_vma(mm, vma)) {
        vma_destroy(vma);
        return -EINVAL;
    }
    if(filep) {
        filep->f_count++;
        vma->vm_file = filep;
        vma->vm_fstart = fstart;
        vma->vm_fsize = fsize;
        vma->onfault = onfault_mmap_file;
    } else {
        vma->onfault = onfault_mmap_anon;
    }
    return 0;
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
        /* find the heap vma */
        struct vm_area *heap = vma_find_region(mm->vmas, mm->start_brk, 0);
        if(!heap)
            kpanic("No heap vm area found!\n");
        if(-1 == vma_grow_up(heap, ALIGN_UP(newbrk, PAGE_SIZE)))
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
 * @return: 1 if added, 0 if not added
 */
int mm_add_vma(struct mm_struct *mm, struct vm_area *vma) {
    struct vm_area *curr;
    if(!mm || !vma || vma->vm_start >= vma->vm_end)
        return -1;

    curr = mm->vmas;
    /* Insert before the first? */
    if(!mm->vmas || vma->vm_end <= curr->vm_end) {
        vma->vm_next = mm->vmas;
        mm->vmas = vma;
        vma->vm_mm = mm;
        mm->vma_count++;
        return 1;
    }
    for(; curr != NULL; curr = curr->vm_next) {
        /* overlapping is bad */
        if(vma_intersects(vma, curr))
            return 0;

        /* After curr? */
        if (curr->vm_end <= vma->vm_start) {
            /* Before next? */
            if(!curr->vm_next || vma->vm_end <= curr->vm_next->vm_start) {
                /* insert here! */
                vma->vm_next = curr->vm_next;
                curr->vm_next = vma;
                vma->vm_mm = mm;
                mm->vma_count++;
                return 1;
            }
        }
    }
    return 0;
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
    if(vma->vm_mm)
        vma->vm_mm->vma_count--;
    if(vma->vm_file)
        vma->vm_file->f_op->close(vma->vm_file); /* fixme: kfree? */
    /* fixme: free all pages in vm_area. CALL free_pagetbl_range_and_pages */

    /* fixme: CAN I FREE vma->vm_file ?? */
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
 * Return 1 if overlapping, 0 if not
 */
int vma_intersects(struct vm_area *vma, struct vm_area *other) {
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
uint64_t onfault_test(struct vm_area *vma, uint64_t addr) {
    char *msg[] = {
        "NOT",
        ""
    };
    debug("0x%lx is %s in the vm_area\n", addr, msg[vma_contains(vma, addr)]);
    //map_page(....)
    return 0;
}

/**
 * Onfault handler for a region with a memory mapped file.
 * @return: 0 on error,
 *          otherwise a kernel virt addr to map to the page table
 */
uint64_t onfault_mmap_file(struct vm_area *vma, uint64_t addr) {
    uint64_t page, aligned, toread;
    off_t offset;
    ssize_t bytes;
    if(!vma)
        kpanic("Null VMA in a page fault!\n");
    if(!vma_contains(vma, addr))
        kpanic("VMA doesn't contain addr %p\n", (void*)addr);
    if(!vma->vm_file)
        kpanic("onfault_mmap_file called, but VMA has no file\n");

    aligned = ALIGN_DOWN(addr, PAGE_SIZE);

    page = get_free_page(0);
    if(!page)
        return 0;

    toread = vma->vm_end - aligned;
    if(toread > PAGE_SIZE)
        toread = PAGE_SIZE;

    offset = vma->vm_fstart + (off_t)(aligned - vma->vm_start);
    bytes = vma->vm_file->f_op->read(vma->vm_file, (char*)page, toread, &offset);
    if(bytes <= 0)
        kpanic("Read error on VMA mmapped file during PF!");
    /* zero extra data, if any */
    memset((void*)(page + bytes), 0, (size_t)PAGE_SIZE - bytes);
    return page;
}

/**
 * Onfault handler for a region with a memory mapped file.
 * @return: 0 on error,
 *          otherwise a kernel virt addr to map to the page table
 */
uint64_t onfault_mmap_anon(struct vm_area *vma, uint64_t addr) {
    uint64_t page;
    if(!vma)
        kpanic("Null VMA in a page fault!\n");
    if(!vma_contains(vma, addr))
        kpanic("VMA doesn't contain addr %p\n", (void*)addr);

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