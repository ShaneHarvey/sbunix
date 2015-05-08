#ifndef SBUNIX_MM_VMM_H
#define SBUNIX_MM_VMM_H

#include <sbunix/sbunix.h>
#include <sbunix/mm/types.h>

/* Start stack for users, maps to pml4[255]->pdpt[511]->pd[511]->pt[511] */
/* TODO: fix this alignment */
#define USER_STACK_START 0x00007ffffffffff8ULL
#define USER_STACK_END   (USER_STACK_START - (20ULL * PAGE_SIZE))

#define USER_MMAP_START

#define USER_HEAP_MAX

/* mm_struct functions */

struct mm_struct *mm_create(void);
void              mm_destroy(struct mm_struct *mm);
struct mm_struct *mm_deep_copy(void);
int               mmap_area(struct mm_struct *mm, struct file *filep,
                            off_t fstart, size_t fsize, uint64_t prot,
                            uint64_t vm_start, uint64_t vm_end);
uint64_t          do_brk(struct mm_struct *mm, uint64_t newbrk);

int add_heap(struct mm_struct *user);
int add_stack(struct mm_struct *user, const char **argv, const char **envp);

/* vm_area functions */

struct vm_area *vma_create(uint64_t vm_start, uint64_t vm_end,
                           vm_type_t type, ulong vm_prot);
void            vma_destroy(struct vm_area *vma);
void            vma_destroy_all(struct mm_struct *mm);
struct vm_area *vma_find_region(struct vm_area *vma, uint64_t addr, size_t size);
struct vm_area *vma_deep_copy(struct mm_struct *mm_old, struct mm_struct *mm_new);
int vma_grow_up(struct vm_area *vma, uint64_t new_end);

/* onfault's */
int onfault_mmap_file(struct vm_area *vma, uint64_t addr);
int onfault_mmap_anon(struct vm_area *vma, uint64_t addr);


/* Also called from the page fault handler. */
int copy_on_write_pagefault(struct vm_area *vma, uint64_t addr);

/* User pointer validation */
int valid_userptr_read(struct mm_struct *mm, const void *userp, size_t size);
int valid_userptr_write(struct mm_struct *mm, void *userp, size_t size);

#endif
