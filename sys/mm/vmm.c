#include <sbunix/sbunix.h>
#include <sbunix/mm/types.h>

/*
 * Virtual Memory for user processes, should be mostly operations on
 * vm_area_struct{}'s
 */

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
        /* fixme: kfree(mm) */
    }
}