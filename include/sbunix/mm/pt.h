#ifndef _SBUNIX_PT_H_
#define _SBUNIX_PT_H_

#include <sys/defs.h>
#define PHYADDRW 48

/* Number of entries in a table */
#define PAGE_ENTRIES 512
#define PAGE_SIZE_4KB  (1<<12)
#define PAGE_SIZE_2MB  (1<<21)
#define PAGE_SIZE_1GB  (1<<30)

#define GET_BITS(x, start, end) (((x) & (~0ULL >> (64 - (end)))) >> (start))

/* Get the page table indexes from a virtual address */
#define PML4_INDEX(va) GET_BITS(va, 39, 48)
#define PDPT_INDEX(va) GET_BITS(va, 30, 39)
#define PD_INDEX(va)   GET_BITS(va, 21, 30)
#define PT_INDEX(va)   GET_BITS(va, 12, 21)

/* Align down to one page */
#define PAGE_ALIGN(ptr) ALIGN_DOWN(ptr, PAGE_SIZE)

/* Get the physical address field of any page entry, stored in 47-12 (36-bits) */
#define PE_PHYS_ADDR(pe) ((uint64_t *)((pe) & 0x0000FFFFFFFFF000UL))
/* Get the flags of the pte, stored in bit 63 and bits 11-0 */
#define PE_FLAGS(pe) ((pe) & 0x8000000000000FFFUL)

/* Get the physical address of the 1GB page frame, stored in 47-30 (18-bits) */
#define PE_PAGE_FRAME_1GB(pe) ((uint64_t *)((pe) & 0x0000FFFFC0000000UL))
/* Get the physical address of the 2MB page frame, stored in 47-21 (27-bits) */
#define PE_PAGE_FRAME_2MB(pe) ((uint64_t *)((pe) & 0x0000FFFFFFE00000UL))
/* Get the physical address of the 2MB page frame, stored in 47-12 (36-bits) */
#define PE_PAGE_FRAME_4KB(pe) PE_PHYS_ADDR(pe)

/* Page flags in the different levels */
#define PFLAG_P   (1UL<<0) /* page is present */
#define PFLAG_RW  (1UL<<1) /* page has write permission */
#define PFLAG_US  (1UL<<2) /* user has permission */
#define PFLAG_PWT (1UL<<3) /* page is write-through, false means write-back */
#define PFLAG_PCD (1UL<<4) /* page will not be cached */
#define PFLAG_A   (1UL<<5) /* CPU has read or written to this PE */
#define PFLAG_D   (1UL<<6) /* CPU has written to this page */
#define PFLAG_PS  (1UL<<7) /* page size */
#define PFLAG_PAT (1UL<<7) /* type of memory to access this page (4KB (PTE's) pages only) */
#define PFLAG_G   (1UL<<8) /* translation is global */
#define PFLAG_NXE (1UL<<63)/* page is not executable */

/* Page Map Level 4 (value of cr3) macros */
#define PML4_WRITE_THROUGH(ptr)  ((ptr) & PFLAG_PWT ) /* True if the PML4 is write-through, false means write-back */
#define PML4_CACHE_DISABLED(ptr) ((ptr) & PFLAG_PCD ) /* True if the PML4 will not be cached */

/* Page Map Level 4 Entry macros */
#define PML4E_PRESENT(ptr)        ((ptr) & PFLAG_P   ) /* True if the PML4E is present */
#define PML4E_WRITE(ptr)          ((ptr) & PFLAG_RW  ) /* True if the PML4E has write permission */
#define PML4E_USER(ptr)           ((ptr) & PFLAG_US  ) /* True if the user has permission */
#define PML4E_WRITE_THROUGH(ptr)  ((ptr) & PFLAG_PWT ) /* True if the PML4E is write-through, false means write-back */
#define PML4E_CACHE_DISABLED(ptr) ((ptr) & PFLAG_PCD ) /* True if the PML4E will not be cached */
#define PML4E_ACCESSED(ptr)       ((ptr) & PFLAG_A   ) /* True if the CPU has read or written to this PML4E */

/* Page Directory Pointer Table Entry macros */
#define PDPTE_PRESENT(ptr)        ((ptr) & PFLAG_P   ) /* True if the PDPTE is present */
#define PDPTE_WRITE(ptr)          ((ptr) & PFLAG_RW  ) /* True if the PDPTE has write permission */
#define PDPTE_USER(ptr)           ((ptr) & PFLAG_US  ) /* True if the user has permission */
#define PDPTE_WRITE_THROUGH(ptr)  ((ptr) & PFLAG_PWT ) /* True if the PDPTE is write-through, false means write-back */
#define PDPTE_CACHE_DISABLED(ptr) ((ptr) & PFLAG_PCD ) /* True if the PDPTE will not be cached */
#define PDPTE_ACCESSED(ptr)       ((ptr) & PFLAG_A   ) /* True if the CPU has read or written to this PDPTE */
#define PDPTE_DIRTY(ptr)          ((ptr) & PFLAG_D   ) /* True if the CPU has written to this 1GB page, NOTE: PS must be 1 */
#define PDPTE_1GB_PAGE(ptr)       ((ptr) & PFLAG_PS  ) /* True if page size is 1GB, false if it's a PD address */
#define PDPTE_GLOBAL(ptr)         ((ptr) & PFLAG_G   ) /* True if the translation is global */

/* Page Directory Entry macros */
#define PDE_PRESENT(ptr)        ((ptr) & PFLAG_P   ) /* True if the PDE is present */
#define PDE_WRITE(ptr)          ((ptr) & PFLAG_RW  ) /* True if the PDE has write permission */
#define PDE_USER(ptr)           ((ptr) & PFLAG_US  ) /* True if the user has permission */
#define PDE_WRITE_THROUGH(ptr)  ((ptr) & PFLAG_PWT ) /* True if the PDE is write-through, false means write-back */
#define PDE_CACHE_DISABLED(ptr) ((ptr) & PFLAG_PCD ) /* True if the page will not be cached */
#define PDE_ACCESSED(ptr)       ((ptr) & PFLAG_A   ) /* True if the CPU has read or written to this PDE */
#define PDE_DIRTY(ptr)          ((ptr) & PFLAG_D   ) /* True if the CPU has written to this 2-MB page, NOTE: PS must be 1 */
#define PDE_2MB_PAGE(ptr)       ((ptr) & PFLAG_PS  ) /* True if page size is 2MB, false if it's a PT address */
#define PDE_GLOBAL(ptr)         ((ptr) & PFLAG_G   ) /* True if the translation is global */

/* Page Table Entry macros */
#define PTE_PRESENT(ptr)        ((ptr) & PFLAG_P   ) /* True if the page is present */
#define PTE_WRITE(ptr)          ((ptr) & PFLAG_RW  ) /* True if the page has write permission */
#define PTE_USER(ptr)           ((ptr) & PFLAG_US  ) /* True if the user has permission */
#define PTE_WRITE_THROUGH(ptr)  ((ptr) & PFLAG_PWT ) /* True if the PDE is write-through, false means write-back */
#define PTE_CACHE_DISABLED(ptr) ((ptr) & PFLAG_PCD ) /* True if the page will not be cached */
#define PTE_ACCESSED(ptr)       ((ptr) & PFLAG_A   ) /* True if the CPU has read or written to this PDE */
#define PTE_DIRTY(ptr)          ((ptr) & PFLAG_D   ) /* True if the CPU has written to this 2-MB page, NOTE: PS must be 1 */
#define PTE_PAT(ptr)            ((ptr) & PFLAG_PAT ) /* Determines type of memory access */
#define PTE_GLOBAL(ptr)         ((ptr) & PFLAG_G   ) /* True if the translation is global */

extern uint64_t kernel_pt;

enum paging_mode {
    pm_none   = 0,
    pm_32_bit = 1,
    pm_pae    = 2,
    pm_ia_32e = 3
};

int get_paging_mode(void);

/**
* Displays the current paging mode to the console
*/
void print_paging_mode(void);

void walk_pages(void);

int map_page(uint64_t virt_addr, uint64_t phy_addr, uint64_t pte_flags);

void init_kernel_pt(uint64_t phys_free_page);

void free_pml4(uint64_t pml4);

uint64_t copy_pml4(uint64_t pml4);
uint64_t copy_current_pml4(void);
uint64_t copy_kernel_pml4(void);

void print_pml4e(void);

void pt_test_map(void);
void pt_test_copying(void);

#endif //_SBUNIX_PT_H_
