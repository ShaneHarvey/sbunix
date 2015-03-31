#ifndef _SBUNIX_PT_H_
#define _SBUNIX_PT_H_

#include <sys/defs.h>
#define PHYADDRW 48

/* Bits 12:47 are the physical address of the 4KB aligned page-directory-pointer table
 * Bits 0:11 and 48:63 are flags/ignored/reserved
 */
#define PML4E(pdpt, flags) (((pdpt) & (((1 << (PHYADDRW - 12)) - 1) << 12)) | flags)

/* See Intel 64 and IA-32 Achitecture Software Developer's Manual Vol. 3A Chapter 4 */
enum pml4eflags {
    PRESENT = (1L << 0),  /* Present bit */
    WRITE   = (1L << 1),  /* Read/write if 0 writes not allowed to 512-GB region */
    USER    = (1L << 2),  /* User/Supervisor: if 0 no user access */
    PWT     = (1L << 3),  /* Page-level write-through; indirectly determines the
                          * memory type used to access the page-directory-pointer
                          * table referenced by this entry (see Section 4.9.2) */
    PCD     = (1L << 4),  /* Page-level cache disable (see Section 4.9.2) */
    XD      = (1L << 63), /* If IA32_EFER.NXE = 1, disable code execution */
};

int get_paging_mode(void);

/**
* Displays the current paging mode to the console
*/
void print_paging_mode(void);
#endif //_SBUNIX_PT_H_
