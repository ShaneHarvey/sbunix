#ifndef SBUNIX_MM_VMM_H
#define SBUNIX_MM_VMM_H

#include <sbunix/sbunix.h>
#include <sbunix/mm/types.h>

/* Start stack for users, maps to top of pml4[479] */
#define USER_STACK_START 0xffffeffffffff000ULL
#define USER_STACK_END   (USER_STACK_START - (20 * PAGE_SIZE))

/* mm_struct functions */

struct mm_struct *mm_create_user(uint64_t start_code, uint64_t end_code,
                                 uint64_t start_data, uint64_t end_data);
struct mm_struct *mm_create(void);
void              mm_destroy(struct mm_struct *mm);
int               mmap_area(struct mm_struct *mm, struct file *filep,
                            off_t fstart, size_t fsize, uint64_t prot,
                            uint64_t vm_start, uint64_t vm_end);
uint64_t          mm_do_sys_sbrk(struct mm_struct *mm, uint64_t newbrk);

int add_heap(struct mm_struct *user);
int add_stack(struct mm_struct *user, char *argv[], char *envp[]);

/* vm_area functions */

struct vm_area *vma_create(uint64_t vm_start, uint64_t vm_end,
                           vm_type_t type, ulong vm_prot);
void            vma_destroy(struct vm_area *vma);
void            vma_destroy_all(struct mm_struct *mm);
struct vm_area *vma_find_region(struct vm_area *vma, uint64_t addr, size_t size);

/* onfault's */
int onfault_mmap_file(struct vm_area *vma, uint64_t addr);
int onfault_mmap_anon(struct vm_area *vma, uint64_t addr);

#endif
