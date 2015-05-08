#include <sbunix/syscall.h>
#include <sbunix/mm/vmm.h>

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