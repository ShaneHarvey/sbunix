#include <sbunix/mm/vmm.h>
#include <sbunix/string.h>

/*
 * Virtual Memory for user processes, should be mostly operations on
 * vm_area_struct's and mm_struct's.
 */


/* Private functions */
int vma_overlaps(struct vm_area_struct *vma, struct vm_area_struct *other);


void mm_struct_destroy(struct mm_struct *mm) {
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
 * Add a vma into the vma list of mm.
 * NOTE: Doesn't add before the head once added.
 */
int mm_add_vma(struct mm_struct *mm, struct vm_area_struct *vma) {
    struct vm_area_struct *curr;
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
 * Allocate and fill a vm_area_struct.
 */
struct vm_area_struct *vma_create(ulong vm_start, ulong vm_end, ulong vm_prot) {
    struct vm_area_struct *vma;
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
 * Return 1 if overlapping, 0 if not
 */
int vma_overlaps(struct vm_area_struct *vma, struct vm_area_struct *other) {
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

