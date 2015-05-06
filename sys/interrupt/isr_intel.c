#include <sbunix/sbunix.h>
#include <sbunix/interrupt/idt.h>
#include <sbunix/asm.h>
#include <sbunix/mm/types.h>
#include <sbunix/mm/vmm.h>
#include <sbunix/sched.h>
#include <sbunix/mm/pt.h>

/**
* This file is for Interrupt Service Routine Handlers for the Reserved
* Intel Interrupts (0 - 31).
*/

/**
* Divide by Zero Exception Handler (#DE)
*/
void ISR_HANDLER(0) {
    kpanic("!! DIVIDE BY ZERO !!\n");
}

void ISR_HANDLER(1) {
    kpanic("!! Debug Exception (#DB) !!\n");
}

void ISR_HANDLER(2) {
    kpanic("!! NMI Interrupt !!\n");
}

void ISR_HANDLER(3) {
    kpanic("!! Breakpoint Exception (#BP) !!\n");
}

void ISR_HANDLER(4) {
    kpanic("!! Overflow Exception (#OF) !!\n");
}

void ISR_HANDLER(5) {
    kpanic("!! BOUND Range Exceeded Exception (#BR) !!\n");
}

void ISR_HANDLER(6) {
    kpanic("!! Invalid Opcode Exception (#UD) !!\n");
}

void ISR_HANDLER(7) {
    kpanic("!! Device Not Available Exception (#NM) !!\n");
}

void ISR_HANDLER(8) {
    kpanic("!! Double Fault Exception (#DF) !!\n");
}

void ISR_HANDLER(9) {
    kpanic("!! Coprocessor Segment Overrun !!\n");
}

void ISR_HANDLER(10) {
    kpanic("!! Invalid TSS Exception (#TS) !!\n");
}

void ISR_HANDLER(11) {
    kpanic("!! Segment Not Present (#NP) !!\n");
}

void ISR_HANDLER(12) {
    kpanic("!! Stack Fault Exception (#SS) !!\n");
}

/**
 * General Protection
 */
void _isr_handler_13(uint64_t errorcode, uint64_t fault_rip) {
    kpanic("General Protection (#GP) at RIP %p, errorcode %p!\n",
           (void*)fault_rip, (void*)errorcode);
}


/* For debugging page faults */
static char *pf_who[] = { "Kernel ", "User "};
static char *pf_read[] = { "read ", "write "};
static char *pf_prot[] = { "non-present", "protection"};
static char *pf_rsvd[] = { "", "reserved "};
static char *pf_inst[] = { "", ", instr fetch "};
/**
* Page fault handler.
*
* @errorcode:   Placed on stack by processor.
*/
void _isr_handler_14(uint64_t errorcode, uint64_t fault_rip) {
    uint64_t addr = read_cr2(); /* read the faulting address */
    uint64_t was_present = (errorcode & PF_PROT);
    uint64_t was_write = (errorcode & PF_WRITE);
    uint64_t was_rsrvd = (errorcode & PF_RSVD);
    debug("%s%s%s%s%s\n",
           pf_who[(errorcode & PF_USER) == PF_USER],
           pf_rsvd[was_rsrvd  == PF_RSVD],
           pf_read[was_write  == PF_WRITE],
           pf_inst[(errorcode & PF_INSTR) == PF_INSTR],
           pf_prot[was_present == PF_PROT]);
    debug("Page-Fault (#PF) at RIP %p, on ADDR %p!\n", (void*)fault_rip, (void*)addr);

    if(was_rsrvd)
        /* We wrote into the reserved bits of a page table entry */
        goto pf_violation;

    if(!was_write && was_present)
        /* We read a page and we didn't have permission. */
        goto pf_violation;

    if(errorcode & PF_USER) {
        /* Find the vm area containing the faulting address */
        struct vm_area *vma;
        vma = vma_find_region(curr_task->mm->vmas, addr, 0);
        if(!vma)
            goto pf_violation;

        /* Normal or Copy-On-Write?
         * The fault was caused by a write on a present page, which is
         * a protection violation.
         */
        if(was_write && was_present) {
            if(vma->vm_prot & PFLAG_RW) {
                /* BUT! The vm area has Read/Write permission. Must be COW! */
                if(copy_on_write_pagefault(vma, addr))
                    goto pf_enomem;
                return;
            } else {
                /* BAD! Tried to write to a present page write permission. */
                goto pf_violation;
            }
        }

        /* Normal Page Fault */
        if(vma->onfault(vma, addr))
            goto pf_enomem;
        return;
    }

pf_violation:
    /* todo: kill self, exit_code = SEGV, call schedule */
    kpanic("!! TODO: Page-Fault SEGV !!\n");
pf_enomem:
    kpanic("!! TODO: Page-Fault ENOMEM !!\n");
    /* todo: Kill self, exit_code = ENOMEM */
}

/* 15 Reserved */

void ISR_HANDLER(16) {
    kpanic("!! x87 FPU Floating-Point Erro (#MF) !!\n");
}

void ISR_HANDLER(17) {
    kpanic("!! Alignment Check Exception (#AC) !!\n");
}

void ISR_HANDLER(18) {
    kpanic("!! Machine-Check Exception (#MC) !!\n");
}

void ISR_HANDLER(19) {
    kpanic("!! SIMD Floating-Point Exception (#XM) !!\n");
}

void ISR_HANDLER(20) {
    kpanic("!! Virtualization Exception (#VE) !!\n");
}

/* 21-31 Reserved */
