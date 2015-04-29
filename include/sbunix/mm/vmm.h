#ifndef SBUNIX_MM_VMM_H
#define SBUNIX_MM_VMM_H

#include <sbunix/sbunix.h>
#include <sbunix/mm/types.h>

/* Start stack for users, maps to pml4[480] */
#define USER_START_STACK 0xfffff00000000000ULL

/* mm_struct functions */

struct mm_struct *mm_create(uint64_t start_code,
                            uint64_t end_code,
                            uint64_t start_data,
                            uint64_t end_data);
void              mm_destroy(struct mm_struct *mm);


/* vm_area functions */

struct vm_area *vma_create(uint64_t vm_start, uint64_t vm_end, ulong vm_prot);
void            vma_destroy(struct vm_area *vma);
struct vm_area *vma_find(struct vm_area *vma, uint64_t addr);


#endif
