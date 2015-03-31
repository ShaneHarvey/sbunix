#include <sbunix/sbunix.h>
#include <sbunix/mm/align.h>
#include <sbunix/mm/pt.h>

#define IS_PAGE_PRESENT(p) ((p) & 1)

enum paging_mode {
    pm_none   = 0,
    pm_32_bit = 1,
    pm_pae    = 2,
    pm_ia_32e = 3
};

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

void walk_pages(void) {
    uint64_t cr3;
    cr3 = read_cr3();
    printf("cr3: %p\n", cr3);
    printf("*cr3: %p\n", *(uint64_t*)cr3);
    printf("**cr3: %p\n", *(uint64_t *)(ALIGN_DOWN(*(uint64_t*)cr3, 4096)));
    printf("***cr3: %p\n", *(uint64_t *)ALIGN_DOWN(*(uint64_t *)(ALIGN_DOWN(*(uint64_t*)cr3, 4096)),4096));

}