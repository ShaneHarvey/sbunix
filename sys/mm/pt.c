#include <sbunix/sbunix.h>
#include <sbunix/mm/align.h>
#include <sbunix/mm/pt.h>
#include <sbunix/string.h>
#include <errno.h>

#define PAGE_SIZE   4096
/* Construct virtual addresses which point to the PTE of a given virtual address */
#define VA_PML4E(va) (uint64_t *)((-1ULL << PHYADDRW) | (GET_BITS(va, 39, 48) << 3) | (pml4_self_index << (PHYADDRW - 9)) | (pml4_self_index << (PHYADDRW - 18)) | (pml4_self_index << (PHYADDRW - 27)) | (pml4_self_index << (PHYADDRW - 36)))
#define VA_PDPTE(va) (uint64_t *)((-1ULL << PHYADDRW) | (GET_BITS(va, 30, 48) << 3) | (pml4_self_index << (PHYADDRW - 9)) | (pml4_self_index << (PHYADDRW - 18)) | (pml4_self_index << (PHYADDRW - 27)))
#define VA_PDE(va)   (uint64_t *)((-1ULL << PHYADDRW) | (GET_BITS(va, 21, 48) << 3) | (pml4_self_index << (PHYADDRW - 9)) | (pml4_self_index << (PHYADDRW - 18)))
#define VA_PTE(va)   (uint64_t *)((-1ULL << PHYADDRW) | (GET_BITS(va, 12, 48) << 3) | (pml4_self_index << (PHYADDRW - 9)))
/**
 * This file sets the cpu in IA-32e Paging mode which enables 64-bit page tables
 */

uint64_t pml4_self_index;
uint64_t kernel_pt;


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
    printk("Paging mode: %s\n", mode);
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
    pt = (uint64_t *)kphys_to_virt((uint64_t)pt);
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
    pd = (uint64_t *)kphys_to_virt((uint64_t)pd);
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
    pdpt = (uint64_t *)kphys_to_virt((uint64_t)pdpt);
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

    pml4 = (uint64_t *)kphys_to_virt((uint64_t)pml4);
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

    if(get_paging_mode() != pm_ia_32e) {
        kpanic("Paging mode is not IA-32e!!!\n");
    }
    if(PML4_CACHE_DISABLED(cr3)) {
        debug("PML4 cache DISABLED\n");
    } else {
        debug("PML4 cache ENABLED\n");
    }
    walk_pml4(PE_PHYS_ADDR(cr3));
}

void map_page_1GB(uint64_t virt_addr, uint64_t phy_addr, uint64_t pte_flags) {
    if(virt_addr != ALIGN_DOWN(virt_addr, 1<<30)) {
        kpanic("Virtual address not on a 1GB boundary: %lx\n", virt_addr);
    }
    if(phy_addr != ALIGN_DOWN(phy_addr, 1<<30)) {
        kpanic("Physical address not on a 1GB boundary: %lx\n", phy_addr);
    }

    pte_flags |= PFLAG_P;
}

static inline int _ensure_present_pde(uint64_t virt_addr) {
    uint64_t *magic = VA_PDE(virt_addr);
    if(!PDE_PRESENT(*magic)) {
        uint64_t pde = get_zero_page();
        if(!pde)
            return 0;
        pde |= PFLAG_RW|PFLAG_US|PFLAG_P;
        debug("Adding PML4[%ld]->PDPT[%ld]->PD[%ld]=0x%lx\n",
              PML4_INDEX(virt_addr), PDPT_INDEX(virt_addr), PD_INDEX(virt_addr), pde);
        *magic = pde;
    }
    return 1;
}

static inline int _ensure_present_pdpte(uint64_t virt_addr) {
    uint64_t *magic = VA_PDPTE(virt_addr);
    if(!PDPTE_PRESENT(*magic)) {
        uint64_t pdpte = get_zero_page();
        if(!pdpte)
            return 0;
        pdpte |= PFLAG_RW|PFLAG_US|PFLAG_P;
        debug("Adding PML4[%ld]->PDPT[%ld]=0x%lx\n", PML4_INDEX(virt_addr),
              PDPT_INDEX(virt_addr), pdpte);
        *magic = pdpte;
    }
    return 1;
}

static inline int _ensure_present_pml4e(uint64_t virt_addr) {
    uint64_t *magic = VA_PML4E(virt_addr);
    if(!PML4E_PRESENT(*magic)) {
        uint64_t pml4e = get_zero_page();
        if(!pml4e)
            return 0;
        pml4e |= PFLAG_RW|PFLAG_US|PFLAG_P;
        debug("Adding PML4[%ld]=0x%lx\n", PML4_INDEX(virt_addr), pml4e);
        *magic = pml4e;
        write_cr3(read_cr3());
    }
    return 1;
}

/**
 * Map a 4KB virtual page to the given 4KB physical page.
 * This maps a page into the current page table  table
 * @virt_addr The virtual address we want to map
 * @phy_addr
 */
int map_page(uint64_t virt_addr, uint64_t phy_addr, uint64_t pte_flags) {
    uint64_t old_pte, *magic;
    /* Should virtual check be done? could be easier on the caller to pass any
     * virtual address and just map to its page.
     */
    if(virt_addr != PAGE_ALIGN(virt_addr)) {
        kpanic("Virtual address not on a page boundary: %lx\n", virt_addr);
    }
    if(phy_addr != PAGE_ALIGN(phy_addr)) {
        kpanic("Physical address not on a page boundary: %lx\n", phy_addr);
    }

    if(!_ensure_present_pml4e(virt_addr) ||
       !_ensure_present_pdpte(virt_addr) ||
       !_ensure_present_pde(virt_addr)) {
        return -ENOMEM;
    }
    /* "Magic" address points to the pte we want to overwrite */
    magic = VA_PTE(virt_addr);
    old_pte = *magic;
    if(PTE_PRESENT(old_pte))
        kpanic("Error: tried to remap present pte 0x%lx\n", old_pte);

    uint64_t new_pte = phy_addr | pte_flags | PFLAG_P;
    debug("Adding PML4[%ld]->PDPT[%ld]->PD[%ld]->PT[%ld]=0x%lx\n",
          PML4_INDEX(virt_addr), PDPT_INDEX(virt_addr), PD_INDEX(virt_addr), PT_INDEX(virt_addr), new_pte);
    *magic = new_pte;
    return 0;
}


/**
 * Sets up the page tables for the kernel in the space after the kernel code.
 * The kernel should have access to all physical addresses and be mapped
 * starting at virt_base.
 * The mapping will be 1-1 starting at virtual_base.
 *
 * @phys_free_page: physical address of the first free page to put the page table
 */
void init_kernel_pt(uint64_t phys_free_page) {
    uint64_t *pml4, *pdpt, *pdt, pdte;
    int i, pml4e_index, pdpte_index;

    if(get_paging_mode() != pm_ia_32e) {
        kpanic("Paging mode is not IA-32e!!!\n");
    }

    pml4 = (uint64_t *)phys_free_page;
    pdpt = (uint64_t *)(phys_free_page + PAGE_SIZE);
    pdt = (uint64_t *)(phys_free_page + 2*PAGE_SIZE);

    pdte = (uint64_t)0|PFLAG_PS|PFLAG_RW|PFLAG_P;
    for(i = 0; i < PAGE_ENTRIES; i++) {
        pml4[i] = 0;
        pdpt[i] = 0;
        pdt[i] = pdte;
        pdte += PAGE_SIZE_2MB;
    }
    pml4e_index = (int)PML4_INDEX(virt_base);
    pdpte_index = (int)PDPT_INDEX(virt_base);
    /* This is the self referencing entry, this lets us modify the page table */
    pml4_self_index = ((pml4e_index - 1) & 0x1FFULL);
    pml4[pml4_self_index] = (uint64_t)pml4|PFLAG_RW|PFLAG_P;

    /* Map virt_base one to one for 1GB at physical address 0x0 */
    pml4[pml4e_index] = (uint64_t)pdpt|PFLAG_RW|PFLAG_P;
    pdpt[pdpte_index] = (uint64_t)pdt|PFLAG_RW|PFLAG_P;

    /* Set CR3 to the pml4 table */
    kernel_pt = (uint64_t)pml4;
    write_cr3(kernel_pt);

    debug("NEW PAGE TABLE! at 0x%lx and 0x%lx\n", pml4, pdpt);
}

/**
 * Recursively free the page table pointed to by pte
 *
 * @level: page table level of pte, 4:PML4, 3:PDPT, 2:PD
 * @pte: page table entry pointing to the table to free
 */
static void rec_free_pt(int level, uint64_t pte) {
    uint64_t *current_pt;  /* The page table pointed to by pte */
    int i;
    if(level > 4 || level < 2)
        kpanic("Invalid call: level cannot be %d\n", level);

    current_pt = (uint64_t *)kphys_to_virt((uint64_t)PE_PHYS_ADDR(pte));

    for(i = 0; i < PAGE_ENTRIES; i++) {
        uint64_t next_pte = current_pt[i];
        /* PML4: skip kernel entry and self-entry */
        if (PTE_PRESENT(next_pte) && !(level == 4 && (i == pml4_self_index || i == PML4_INDEX(virt_base)))) {
            if(level == 2){
                /* Level2:PD, means next_pte is a Page Table so just free it */
                free_page(kphys_to_virt((uint64_t)PE_PHYS_ADDR(next_pte)));
            } else {
                /* Level 3 or 4: recursively go down and free */
                rec_free_pt(level - 1, next_pte);
            }
        }
    }
    /* Finally free the current page table */
    free_page((uint64_t)current_pt);
}

/**
 * Free the given PML4 table
 *
 * @pml4: physical address of the PML4 table to free
 */
void free_pml4(uint64_t pml4) {
    rec_free_pt(4, pml4);
}

/**
 * Recursively copy the page table pointed to by pte
 *
 * @level: page table level of pte, 4:PML4, 3:PDPT, 2:PD, 1:PT
 * @pte: page table entry pointing to the table to free
 */
static uint64_t rec_copy_pt(int level, uint64_t pte) {
    uint64_t *new_pt;
    uint64_t *current_pt;  /* page table pointed to by pte */
    int i;
    if(level > 4 || level < 1)
        kpanic("Invalid call: level cannot be %d\n", level);

    new_pt = (uint64_t *)get_free_page(0);
    if(!new_pt) {
        debug("get_free_page: failed! at level=%d, pte=0x%lx\n", level, pte);
        return 0;
    }
    memset(new_pt, 0, PAGE_SIZE);

    current_pt = (uint64_t *)kphys_to_virt((uint64_t)PE_PHYS_ADDR(pte));

    for(i = 0; i < PAGE_ENTRIES; i++) {
        uint64_t other_pte = current_pt[i];

        /* PML4: skip kernel entry and self-entry */
        if (PTE_PRESENT(other_pte)) {
            if(level == 1 || (level == 4 && i == PML4_INDEX(virt_base)) ) {
                /* Add it to Page Table level 1 */
                new_pt[i] = other_pte;
            } else if (level == 4 && i == pml4_self_index) {
                new_pt[i] = kvirt_to_phys((uint64_t)new_pt)|PFLAG_RW|PFLAG_P; /* add self index */
            } else {
                /* Level 2, 3 or 4: recursively go down and copy */
                uint64_t new_pte = rec_copy_pt(level - 1, other_pte);
                if(!new_pte)
                    goto error_copy;
                new_pt[i] = new_pte;
            }
        } else {
            /* Mark as not present */
            new_pt[i] = 0;
        }
    }

    /* Keep the same flags */
    return kvirt_to_phys((uint64_t)new_pt) | PE_FLAGS(pte);
error_copy:
    /* free all the pages we allocated for the current page */
    debug("error_copy: level=%d, new_pt=0x%lx\n", level, new_pt);
    rec_free_pt(level, kvirt_to_phys((uint64_t)new_pt));
    return 0;
}

/**
 * Copy the given PML4 table
 *
 * @pml4: physical address of the PML4 table to copy
 */
uint64_t copy_pml4(uint64_t pml4) {
    return rec_copy_pt(4, pml4);
}

/**
 * Copy the currently used PML4 table
 */
uint64_t copy_current_pml4(void) {
    return rec_copy_pt(4, read_cr3());
}

/**
 * Create a copy of the kernel page table entries
 *
 * @return: physical address of a new PML4 table with the kernel page table entries
 */
uint64_t copy_kernel_pml4(void) {
    uint64_t *pml4;
    uint64_t *virt_kern_pt;
    int i;

    pml4 = (uint64_t *)get_free_page(0);
    if(!pml4)
        return 0;

    virt_kern_pt = (uint64_t *)kphys_to_virt(kernel_pt);

    for(i = 0; i < PAGE_ENTRIES; i++) {
        pml4[i] = virt_kern_pt[i];
    }
    /* Add PLM4E entry to self */
    pml4[pml4_self_index] = kvirt_to_phys((uint64_t)pml4)|PFLAG_RW|PFLAG_P;

    return kvirt_to_phys((uint64_t)pml4) | PE_FLAGS(kernel_pt);
}


/**
 * Prints the current page table's PML4 entries
 */
void print_pml4e(void) {
    int i;
    uint64_t *pml4 = (uint64_t *)kphys_to_virt(read_cr3());
    printk("PML4 Entries:");
    for(i = 0; i < PAGE_ENTRIES; i++) {
        if(PML4E_PRESENT(pml4[i])){
            printk("%d: 0x%lx,", i, pml4[i]);
        }
    }
}

/**
 * Small test of mapping a page into the current address space
 */
void pt_test_map(void) {
    /* Quick test of map_page */
    uint64_t phys_page = get_zero_page();
    uint64_t va = 0x000000001000;
    debug("Attempting to map physa 0x%lx to va 0x%lx\n", phys_page, va);
    if(!map_page(va, phys_page, PFLAG_RW)) {
        debug("Mapped virtual 0x%lx to physical 0x%lx, data 0x%lx\n", va, phys_page, *(uint64_t *)va);
    } else {
        kpanic("Failed to map!\n");
    }
}

/**
 * Test copying/freeing page tables
 */
void pt_test_copying(void) {
    uint64_t new_pml4;
    int i;
    debug("Starting page tage copying test...\n");
    freemem_report();
    for(i = 0; i < 10; i++) {
        new_pml4 = copy_current_pml4();
        if(new_pml4) {
            free_pml4(new_pml4);
        } else {
            debug("copy_current_pml4: ENOMEM!!\n");
        }
    }
    freemem_report();
    debug("End page tage copying test\n");
}
