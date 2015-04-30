#ifndef SBUNIX_MM_VMM_H
#define SBUNIX_MM_VMM_H

#include <sbunix/sbunix.h>
#include <sbunix/mm/types.h>

/* Start stack for users, maps to pml4[480] */
#define USER_STACK_START 0xfffff00000000000ULL
#define USER_STACK_END   (USER_STACK_START - (20 * PAGE_SIZE))

/* mm_struct functions */

struct mm_struct *mm_create_user(uint64_t start_code, uint64_t end_code,
                                 uint64_t start_rodata, uint64_t end_rodata,
                                 uint64_t start_data, uint64_t end_data);
struct mm_struct *mm_create(void);
void              mm_destroy(struct mm_struct *mm);
int               mmap_area(struct mm_struct *mm, struct file *filep,
                            off_t fstart, ulong prot, vm_type_t type,
                            uint64_t vm_start, uint64_t vm_end,
                            uint64_t (*onfault) (struct vm_area *, uint64_t));
uint64_t          mm_do_sys_sbrk(struct mm_struct *mm, uint64_t newbrk);


/* vm_area functions */

struct vm_area *vma_create(uint64_t vm_start, uint64_t vm_end,
                           vm_type_t type, ulong vm_prot);
void            vma_destroy(struct vm_area *vma);
void            vma_destroy_all(struct mm_struct *mm);
struct vm_area *vma_find_region(struct vm_area *vma, uint64_t addr, size_t size);
struct vm_area *vma_find_type(struct vm_area *vma, vm_type_t type);


#endif
