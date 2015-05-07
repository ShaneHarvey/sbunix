#include <sbunix/mm/vmm.h>
#include <sbunix/mm/pt.h>
#include <sbunix/fs/vfs.h>
#include <sbunix/string.h>
#include <sbunix/sched.h>
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
 * Adds a heap vm area above the last currently in user_mm's vm areas.
 * Called after loading loadable segments from the ELF file.
 * @return: 0 on success, -errno on failure
 */
int add_heap(struct mm_struct *user) {
    struct vm_area *vma, *heap;
    if(!user || !user->vmas)
        return -EINVAL;
    for(vma = user->vmas; vma->vm_next != NULL; vma = vma->vm_next) {
    }
    /* Heap starts after the other sections */
    user->start_brk = user->brk = ALIGN_UP(vma->vm_end + 1, PAGE_SIZE);
    /* Initially size is 0 */
    heap = vma_create(user->start_brk, user->brk, VM_HEAP, PFLAG_RW);
    heap->onfault = onfault_mmap_anon;
    heap->vm_mm = user;
    user->vma_count++;
    vma->vm_next = heap;
    return 0;
}


/**
 * Check that argv and envp are valid, and validate
 * E2BIG: they're too big to fit in 2 pages
 */

int num_pointers(const char **array) {
    int len = 0;
    if(!array)
        return 0;
    while(*array++)
        len++;
    return len;
}

int num_bytes(const char **array) {
    int bytes = 0, err = 0;
    if(!array)
        return 0;
    while(*array) {
        size_t len = strnlen(*array, PAGE_SIZE) + 1;   /* TODO: this could fault */
        /* TODO: testing exec... uncomment for security */
//        err = valid_userptr_read(curr_task->mm, *array, len);
        if(err)
            return err;

        bytes += len;
        array++;
    }
    return bytes;
}

#define MAX_ARG_ENV_BYTES PAGE_SIZE
/* gotta leave room for the actual stack */
#define MAX_ARG_ENV_PTRS ((PAGE_SIZE/sizeof(void *)) - 10)

int argv_envp_err(const char **argv, const char **envp, int *argc, int *envc) {
    int arg_bytes, env_bytes;

    /* first iteration argv, second envp */
    *argc = num_pointers(argv);
    *envc = num_pointers(envp);
    arg_bytes = num_bytes(argv);
    if(arg_bytes < 0)
        return arg_bytes;

    env_bytes = num_bytes(envp);
    if(env_bytes < 0)
        return env_bytes;

    if((arg_bytes + env_bytes) > MAX_ARG_ENV_BYTES ||
            (*argc + *envc + 2) > MAX_ARG_ENV_PTRS)
        return -E2BIG;
    return 0;
}


/**
 * Copy num strings from strs into virt_strs, put user pointers in virt_ptrs
 *
 * copy backwards from last string
 */
void copy_strings(const char **strs, int num, uint64_t *ptrs, char **deststrsp, int off) {
    uint64_t user_ptr = PAGE_ALIGN(USER_STACK_START);
    char *dest = *deststrsp;
    user_ptr += (uint64_t)dest - PAGE_ALIGN((uint64_t)dest);

    ptrs[--off] = 0; /* start from end */
    while(num--) {
        const char *src = strs[num];
        ptrs[--off] = user_ptr; /* user pointer to dest string */
        while (user_ptr++ && (*dest++ = *src++) != '\0')/* nothing */;
    }
    *deststrsp = dest;
}

/**
 * Add the stack vm area.
 *
 * TODO: I don't know why I stayed up doing this. I can't think
 */
int add_stack(struct mm_struct *user, const char **argv, const char **envp) {
    struct vm_area *stack;
    uint64_t phys_ptrs, phys_strs, *virt_ptrs;
    char *virt_strs; /* hold page of user strings */
    int err, argc, envc;

    user->start_stack = USER_STACK_START;
    stack = vma_create(USER_STACK_END, USER_STACK_START, VM_STACK, PFLAG_RW);
    if(!stack)
        return -ENOMEM;
    stack->onfault = onfault_mmap_anon;

    /* Copy envp and args to new stack */
    phys_ptrs = get_zero_page();
    if(!phys_ptrs) {
        err = -ENOMEM;
        goto out_vma;
    }
    virt_ptrs = (uint64_t *)kphys_to_virt(phys_ptrs);
    phys_strs = get_zero_page();
    if(!phys_strs) {
        err = -ENOMEM;
        goto out_virt_ptrs;
    }
    virt_strs = (char *)kphys_to_virt(phys_strs);

    err = argv_envp_err(argv, envp, &argc, &envc); /* TODO: move this? validate pointers */
    if(err)
        goto out_virt_strs;
    /* Now safe to copy! */
    copy_strings(envp, envc, virt_ptrs, &virt_strs, 512);
    copy_strings(argv, argc, virt_ptrs, &virt_strs, 512 - envc - 1);
    virt_ptrs[512 - (argc + envc + 3)] = (int64_t)argc;
    /* Map the string page to the top */
    err = map_page_into(ALIGN_DOWN(USER_STACK_START, PAGE_SIZE), phys_strs,
                        stack->vm_prot, user->pml4);
    if(err)
        goto out_virt_strs;
    /* Map the pointer page next and set entry rsp */
    user->user_rsp = USER_STACK_START - PAGE_SIZE - (8 * (argc + envc + 2));
    err = map_page_into(ALIGN_DOWN(user->user_rsp, PAGE_SIZE), phys_ptrs,
                        stack->vm_prot, user->pml4);
    if(err)
        goto out_virt_strs;

    /* Finally, add stack to the user */
    if(mm_add_vma(user, stack)) {
        err = -ENOEXEC;
        goto out_vma;
    }

    return 0;
out_virt_strs:
    free_page((uint64_t)virt_strs);
out_virt_ptrs:
    free_page((uint64_t)virt_ptrs);
out_vma:
    vma_destroy(stack);
    return err;
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
              off_t fstart, size_t fsize, uint64_t prot,
              uint64_t vm_start, uint64_t vm_end) {

    struct vm_area *vma;
    int err;
    uint64_t curr_pml4;
    if(!mm)
        return -EINVAL;

    vma = vma_create(vm_start, vm_end, VM_MMAP, prot);
    if(!vma)
        return -ENOMEM;
    if(mm_add_vma(mm, vma)) {
        err = -EINVAL;
        goto out_vma;
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
    /* pre-fault the first page */
    curr_pml4 = read_cr3();
    write_cr3(mm->pml4);
    err = vma->onfault(vma, vm_start);
    write_cr3(curr_pml4);
    if(err){
        goto out_vma;
    }
    return 0;

out_vma:
    vma_destroy(vma);
    return err;
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
    /* Create a copy of the kernel page tables */
    mm->pml4 = copy_kernel_pml4();
    if(mm->pml4 == 0) {
        kfree(mm);
        return NULL;
    }
    mm->mm_count = 1;
    mm_list_add(mm);
    return mm;
}

/**
 * Decrement the reference count of the mm, if mm_count goes to 0:
 *      * Free all the vma's and then mm struct itself.
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

        /* Free the page_tables, if it exists (could come here during a
         * mm_deep_copy error) */
        if(mm->pml4)
            free_pml4(mm->pml4);

        kfree(mm);
    }
}

/**
 * Return a deep copy of the current task's mm_struct.
 * This is used by fork.
 */
struct mm_struct *mm_deep_copy(void) {
    struct mm_struct *curr_mm = curr_task->mm;
    struct mm_struct *copy_mm;
    if(!curr_mm)
        kpanic("Current task has no memory struct!\n");

    if(curr_task->type == TASK_KERN)
        kpanic("Trying to fork a kernel mm\n");

    copy_mm = kmalloc(sizeof(*copy_mm));
    if(!copy_mm)
        return NULL;

    /* Copy exactly from parent */
    memcpy(copy_mm, curr_mm, sizeof(*copy_mm));
    /* set pml4 to NULL so we don't free the parent's */
    curr_mm->pml4 = 0;
    /* Update the prev/next mm pointers */
    mm_list_add(copy_mm);

    /* Copy all of the vma's, incrementing file ref counts */
    copy_mm->vmas = vma_deep_copy(curr_mm, copy_mm);
    if(!copy_mm->vmas)
        goto out_copy_mm;

    /* Create a copy of the page tables that are now Copy-On-Write */
    curr_mm->pml4 = copy_current_pml4();
    if(!curr_mm->pml4)
        goto out_copy_mm;

    return copy_mm;
out_copy_mm:
    /* Destroy the vma's if any and the copy_mm */
    mm_destroy(copy_mm);
    return NULL;
}


/**
 * Real work for brk()
 *    new break     -- on success
 *    current break -- on failure
 */
uint64_t do_brk(struct mm_struct *mm, uint64_t newbrk) {
    if(!mm)
        kpanic("Null mm in do_brk!\n");
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
 * @return: 0 if added, -1 on error.
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
        return 0;
    }
    for(; curr != NULL; curr = curr->vm_next) {
        /* overlapping is bad */
        if(vma_intersects(vma, curr))
            return -1;

        /* After curr? */
        if (curr->vm_end <= vma->vm_start) {
            /* Before next? */
            if(!curr->vm_next || vma->vm_end <= curr->vm_next->vm_start) {
                /* insert here! */
                vma->vm_next = curr->vm_next;
                curr->vm_next = vma;
                vma->vm_mm = mm;
                mm->vma_count++;
                return 0;
            }
        }
    }
    return -1;
}

/**
 * Checks if the user pointer is within a valid virtual memory area.
 * Syscalls set the global in a syscall flag used in the page fault
 * handler.
 * This does NOT map pages!
 * @return:  0 if userp is valid and is safe to page fault in the kernel, or
 *          -EFAULT if not valid
 */
int valid_userptr_read(struct mm_struct *mm, const void *userp, size_t size) {
    struct vm_area *vma;

    vma = vma_find_region(mm->vmas, (uint64_t)userp, size);
    if(!vma)
        return -EFAULT;  /* not valid */

    return 0;
}

/**
 * Checks if the user pointer is write-able within a vma
 * This does NOT map pages!
 * @return:  0 if userp is valid and is safe to page fault in the kernel, or
 *          -EFAULT if not valid
 */
int valid_userptr_write(struct mm_struct *mm, void *userp, size_t size) {
    struct vm_area *vma;

    vma = vma_find_region(mm->vmas, (uint64_t)userp, size);

    if(vma && vma->vm_prot & PFLAG_RW)
        return 0; /* exists and is write-able */

    return -EFAULT;
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
    vma->vm_prot = vm_prot | PFLAG_US;
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
        vma->vm_file->f_op->close(vma->vm_file);

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
 * Return a deep copy of the vm_areas of the mm struct.
 * Copying the vm_area's does not increment the mapcounts for the physical
 * pages used in the regions.
 * @return: return the first vma in a list of vma's
 */
struct vm_area *vma_deep_copy(struct mm_struct *mm_old, struct mm_struct *mm_new) {
    int firstvma_set = 0;
    struct vm_area *firstvma = NULL;
    struct vm_area *prevnew = NULL, *new = NULL, *old;

    if(!mm_old)
        kpanic("Null mm in vma_deep_copy\n");

    old = mm_old->vmas;
    for(; old != NULL; old = old->vm_next){
        new = kmalloc(sizeof(*new));
        if(!new)
            goto out_vmas;

        /* Create an exact copy, then change pointers */
        memcpy(new, old, sizeof(*old));

        if(!firstvma_set) {
            /* Keep the reference to the first vma copied (to return) */
            firstvma = new;
            firstvma_set = 1;
        }

        new->vm_mm = mm_new; /* new mm owner */
        new->vm_next = NULL;
        if(new->vm_file)
            new->vm_file->f_count++; /* inc ref count */

        if(prevnew)
            prevnew->vm_next = new; /* build list of new vm_areas */

        prevnew = new;
    }
    return firstvma;

out_vmas:
    /* free all vmas here */
    while(firstvma != NULL) {
        struct vm_area *prev = firstvma;
        firstvma = firstvma->vm_next;

        if(prev->vm_file)
            prev->vm_file->f_count--; /* undo ref count inc */
        kfree(prev);
    }
    return NULL;
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
    if(vma->vm_start == vma->vm_end)
        return vma->vm_start == addr;
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
 * Onfault handler for a region with a memory mapped file.
 * @return: error or 0, same as map_page
 */
int onfault_mmap_file(struct vm_area *vma, uint64_t addr) {
    uint64_t page, aligned, toread;
    off_t offset;
    ssize_t bytes;
    if(!vma)
        kpanic("Null VMA in a page fault!\n");
    if(!vma_contains(vma, addr))
        kpanic("VMA doesn't contain addr %p\n", (void*)addr);
    if(!vma->vm_file)
        kpanic("onfault_mmap_file called, but VMA has no file\n");

    page = get_free_page(0);
    if(!page)
        return -ENOMEM;

    aligned = ALIGN_DOWN(addr, PAGE_SIZE);
    if(aligned > vma->vm_start + vma->vm_fsize) {
        /* ANON */
        memset((void*)page, 0, PAGE_SIZE);
    } else if(aligned < vma->vm_start) {
        /* Mapping the first page, vm_start may not be page aligned */
        uint64_t diff = vma->vm_start - aligned;
        toread = MIN(PAGE_SIZE - diff, vma->vm_fsize);
        offset = vma->vm_fstart;
        bytes = vma->vm_file->f_op->read(vma->vm_file, (char*)page + diff, toread, &offset);
        if(bytes <= 0)
            kpanic("Read error on VMA mmapped file during PF!");
        debug("Read  0x%lx/0x%lx bytes, new off_t 0x%lx\n", bytes,toread, (uint64_t)offset);
        /* zero extra data, if any */
        memset((void*)(page + bytes + diff), 0, (size_t)PAGE_SIZE - (bytes + diff));
    } else {
        /* read from file (still account for boundary being within this page) */
        toread = (vma->vm_start + vma->vm_fsize) - aligned;
        if(toread > PAGE_SIZE)
            toread = PAGE_SIZE;

        offset = vma->vm_fstart + (off_t)(aligned - vma->vm_start);
        bytes = vma->vm_file->f_op->read(vma->vm_file, (char*)page, toread, &offset);
        if(bytes <= 0)
            kpanic("Read error on VMA mmapped file during PF!");
        debug("Read  0x%lx/0x%lx bytes, new off_t 0x%lx\n", bytes,toread, (uint64_t)offset);
        /* zero extra data, if any */
        memset((void*)(page + bytes), 0, (size_t)PAGE_SIZE - bytes);
    }

    return map_page(aligned, kvirt_to_phys(page), vma->vm_prot);
}

/**
 * Onfault handler for a region with a memory mapped file.
 * @return: error or 0, same as map_page
 */
int onfault_mmap_anon(struct vm_area *vma, uint64_t addr) {
    uint64_t physpage, aligned;
    if(!vma)
        kpanic("Null VMA in a page fault!\n");
    if(!vma_contains(vma, addr))
        kpanic("VMA doesn't contain addr %p\n", (void*)addr);

    physpage = get_zero_page();
    if(!physpage)
        return -ENOMEM;
    aligned = ALIGN_DOWN(addr, PAGE_SIZE);

    return map_page(aligned, physpage, vma->vm_prot);
}


/**
 * For a Copy-On-Write page fault.
 * NOTE: This is only called when we have faulted on a PRESENT page.
 * @vma: the vma that contains addr
 * @addr: user virtual fault address
 */
int copy_on_write_pagefault(struct vm_area *vma, uint64_t addr) {
    uint64_t aligned = ALIGN_DOWN(addr, PAGE_SIZE);
    return copy_cow_page(aligned, vma->vm_prot);
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