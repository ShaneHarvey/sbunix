#include <sbunix/sbunix.h>
#include <sbunix/mm/align.h>
#include <sbunix/mm/pt.h>

#define VIRTUAL_BASE 0xffffffff80000000UL
#define PAGE_SIZE   4096

/**
* This file sets the cpu in IA-32e Paging mode which enables 64-bit page tables
*/

/**
* Determines the current paging mode of the CPU
* For info on CR* register formats see section 2.5
* For info on sequence of checks see section 4.1.1
*/
int get_paging_mode(void) {
    uint64_t cr0, cr4, ia32_efer;
    cr0 = read_cr0();
    /* Check PG (bit 31) of CR0 to see if paging is enabled */
    if(!((cr0 >> 31) & 1)){
        return pm_none;
    }
    cr4 = read_cr4();
    /* Check PAE (bit 5) of CR4 to see if PAE is enabled */
    if(!((cr4 >> 5) & 1)){
        return pm_32_bit;
    }
    ia32_efer = rdmsr(0xC0000080);
    /* Check LME (IA-32e Mode Enabled) (bit 8) of IA32_EFER (MSR C0000080H) */
    if(!((ia32_efer >> 8) & 1)){
        return pm_pae;
    }
    return pm_ia_32e;
}

/**
* Displays the current paging mode to the console
*/
void print_paging_mode(void) {
    int pm = get_paging_mode();
    char *mode;
    switch (pm) {
        case pm_none:
            mode = "none";
            break;
        case pm_32_bit:
            mode = "32-bit";
            break;
        case pm_pae:
            mode = "PAE";
            break;
        case pm_ia_32e:
            mode = "IA-32e";
            break;
        default:
            mode = "unknown!";
    }
    printf("Paging mode: %s\n", mode);
}

/**
* NOTE: This code assumes the current page tables are 1-1 virtual-to-physical
*
* Traverse each entry in the Page Table
*
* @pt: The PD entry corresponding to this PT
*/
void walk_pt(uint64_t* pt) {
    int index;
    for(index = 0; index < PAGE_ENTRIES; index++) {
        uint64_t pte = pt[index];

        /* Print flags of this pml4 entry */

        if(PDE_PRESENT(pte)) {
            /* Print info */
        }
    }
}

/**
* NOTE: This code assumes the current page tables are 1-1 virtual-to-physical
*
* Traverse each entry in the Page Directory
*
* @pd: The PDPT entry corresponding to this PD
*/
void walk_pd(uint64_t* pd) {
    int index;
    for(index = 0; index < PAGE_ENTRIES; index++) {
        uint64_t pde = pd[index];

        /* Print flags of this pml4 entry */

        if(PDE_PRESENT(pde)) {
            if(PDE_2MB_PAGE(pde)) {
                debug("PDE_2MB_PAGE\n");
            } else {
                /* pde holds the address of a Page Table */
                walk_pt(PE_PHYS_ADDR(pde));
            }
        }
    }
}

/**
* NOTE: This code assumes the current page tables are 1-1 virtual-to-physical
*
* Traverse each entry in the Page Directory Pointer Table
*
* @pdpt: The PML4 entry corresponding to this PDPT
*/
void walk_pdpt(uint64_t* pdpt) {
    int index;
    for(index = 0; index < PAGE_ENTRIES; index++) {
        uint64_t pdpte = pdpt[index];

        /* Print flags of this pml4 entry */

        if(PDPTE_PRESENT(pdpte)) {
            if(PDPTE_1GB_PAGE(pdpte)) {
                debug("PDPTE_1GB_PAGE\n");
            } else {
                /* pdpte holds the address of a Page Directory */
                walk_pd(PE_PHYS_ADDR(pdpte));
            }
        }
    }
}

/**
* NOTE: This code assumes the current page tables are 1-1 virtual-to-physical
*
* Traverse each entry in the Page Map level-4 Table pointed to by the physical
* address in cr3
*
* @pml4: The physical address of the PML4 page table
*/
void walk_pml4(uint64_t *pml4) {
    int index;
    if(get_paging_mode() != pm_ia_32e) {
        kpanic("Paging mode is not IA-32e!!!\n");
    }
    for(index = 0; index < PAGE_ENTRIES; index++) {
        uint64_t pml4e = pml4[index];

        /* Print flags of this pml4 entry */

        if(PML4E_PRESENT(pml4e)) {
            walk_pdpt(PE_PHYS_ADDR(pml4e));
        }
    }
}

/**
* This was a exercise to better understand the IA-32e paging mode and should
* probably never be called.
*
* Walk through each present entry in the page table
*/
void walk_pages(void) {
    uint64_t cr3 = read_cr3();

    if(PML4_CACHE_DISABLED(cr3)) {
        debug("PML4 cache DISABLED\n");
    } else {
        debug("PML4 cache ENABLED\n");
    }
    walk_pml4(PE_PHYS_ADDR(cr3));
}

void map_frame_1GB(uint64_t virt_addr, uint64_t phy_addr, uint64_t pte_flags) {
    if(virt_addr != ALIGN_DOWN(virt_addr, 1<<30)) {
        kpanic("Virtual address not on a 1GB boundary: %lx\n", virt_addr);
    }
    if(phy_addr != ALIGN_DOWN(phy_addr, 1<<30)) {
        kpanic("Physical address not on a 1GB boundary: %lx\n", phy_addr);
    }

    pte_flags |= PFLAG_P;
}

/**
 * Sets up the page tables for the kernel in the space after the kernel code.
 * The kernel should have access to all physical addresses and be mapped into
 * the high address space. 0xffffffff80000000
 *
 * The mapping will be 1-1 starting at virtual_base.
 * To map a physical address to virtual just add virtual_base
 * To map a virtual address to physical just subtract virtual_base
 *
 * @phys_free_page: physical address of the first free page to put the page table
 */
void init_kernel_pt(uint64_t phys_free_page) {
    uint64_t *pml4, *pdpt;
    int i;

    if(get_paging_mode() != pm_ia_32e) {
        kpanic("Paging mode is not IA-32e!!!\n");
    }

    pml4 = (uint64_t *)phys_free_page;
    pdpt = (uint64_t *)(phys_free_page + PAGE_SIZE);

    for(i = 0; i < PAGE_ENTRIES; i++) {
        pml4[i] = PFLAG_RW;
    }
    for(i = 0; i < PAGE_ENTRIES; i++) {
        pdpt[i] = PFLAG_RW;
    }

    /* Map 0xffffffff80000000 to the 1GB physical page starting at 0x0 */
    pml4[511] = (uint64_t)pdpt|PFLAG_RW|PFLAG_P;
    pdpt[510] = (uint64_t)0|PFLAG_PS|PFLAG_RW|PFLAG_P;

    /* Set CR3 to the pml4 table */
    __asm__ __volatile__ ("movq %0, %%cr3;"::"g"(pml4));

    printf("NEW PAGE TABLE! at 0x%lx and 0x%lx\n", pml4, pdpt);
}
