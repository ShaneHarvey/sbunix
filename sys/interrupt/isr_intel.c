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
    /* Divide by 0 is bad */
    kill_curr_task(EXIT_FATALSIG + SIGFPE);
    kpanic("!! DIVIDE BY ZERO !!\n");
}

void ISR_HANDLER(1) {
    /* Hardware debugging is not allowed */
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
    kpanic("!! Debug Exception (#DB) !!\n");
}

void ISR_HANDLER(2) {
    printk("!! NMI Interrupt, H/W Failure? Will resume... !!\n");
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
}

void ISR_HANDLER(3) {
    /* The INT3 ? instruction is not allowed */
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
    kpanic("!! Breakpoint Exception (#BP) !!\n");
}

void ISR_HANDLER(4) {
    /* The INTO instruction (traps on overflow) is not allowed */
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
    kpanic("!! Overflow Exception (#OF) !!\n");
}

void ISR_HANDLER(5) {
    /* The BOUND instruction is not allowed */
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
    kpanic("!! BOUND Range Exceeded Exception (#BR) !!\n");
}

void ISR_HANDLER(6) {
    kill_curr_task(EXIT_FATALSIG + SIGILL);
    kpanic("!! Invalid Opcode Exception (#UD) !!\n");
}

void ISR_HANDLER(7) {
    /* Attempt to use the FPU, we don't allow floats (-msoft-float -mno-sse) */
    kill_curr_task(EXIT_FATALSIG + SIGILL);
    kpanic("!! Device Not Available Exception (#NM) !!\n");
}

void ISR_HANDLER(8) {
    kill_curr_task(EXIT_FATALSIG + SIGILL);
    kpanic("!! Double Fault Exception (#DF) !!\n");
}

void ISR_HANDLER(9) {
    kill_curr_task(EXIT_FATALSIG + SIGILL);
    kpanic("!! Coprocessor Segment Overrun !!\n");
}

void ISR_HANDLER(10) {
    kpanic("!! Invalid TSS Exception (#TS) !!\n");
}

void ISR_HANDLER(11) {
    kill_curr_task(EXIT_FATALSIG + SIGILL);
    kpanic("!! Segment Not Present (#NP) !!\n");
}

void ISR_HANDLER(12) {
    kill_curr_task(EXIT_FATALSIG + SIGILL);
    kpanic("!! Stack Fault Exception (#SS) !!\n");
}

/**
 * General Protection
 */
void _isr_handler_13(uint64_t errorcode, uint64_t fault_rip) {
    kill_curr_task(EXIT_FATALSIG + SIGILL);
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
    uint64_t was_user = (errorcode & PF_USER);
    debug("%s%s%s%s%s\n",
           pf_who[was_user == PF_USER],
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

    /* If we're a user, or we are in a system call for a user. */
    if(was_user || curr_task->in_syscall) {
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

    /* Reaching here is a Kernel OOPS */
    printk("!! %s%s%s%s%s !!\n",
           pf_who[was_user == PF_USER],
           pf_rsvd[was_rsrvd  == PF_RSVD],
           pf_read[was_write  == PF_WRITE],
           pf_inst[(errorcode & PF_INSTR) == PF_INSTR],
           pf_prot[was_present == PF_PROT]);
    printk("Page-Fault (#PF) at RIP %p, on ADDR %p!\n", (void*)fault_rip, (void*)addr);
    kpanic("KERNEL OOPS please reboot\n");

pf_violation:
    debug("Page-Fault (#PF) at RIP %p, on ADDR %p!\n", (void*)fault_rip, (void*)addr);
    /* Kill current task */
    kill_curr_task(EXIT_FATALSIG + SIGSEGV);
    kpanic("!! kill_curr_task returned!! Page-Fault SEGV !!\n");

pf_enomem:
    debug("Page-Fault (#PF) at RIP %p, on ADDR %p!\n", (void*)fault_rip, (void*)addr);
    /* Kill current task */
    kill_curr_task(EXIT_ENOMEM);
    kpanic("!! TODO: Page-Fault ENOMEM !!\n");
}

/* 15 Reserved */

void ISR_HANDLER(16) {
    kill_curr_task(EXIT_FATALSIG + SIGFPE);
    kpanic("!! x87 FPU Floating-Point Exception (#MF) !!\n");
}

void ISR_HANDLER(17) {
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
    kpanic("!! Alignment Check Exception (#AC) !!\n");
}

void ISR_HANDLER(18) {
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
    kpanic("!! Machine-Check Exception (#MC) !!\n");
}

void ISR_HANDLER(19) {
    kill_curr_task(EXIT_FATALSIG + SIGFPE);
    kpanic("!! SIMD Floating-Point Exception (#XM) !!\n");
}

void ISR_HANDLER(20) {
    /* Virtualization is not allowed */
    kill_curr_task(EXIT_FATALSIG + SIGKILL);
    kpanic("!! Virtualization Exception (#VE) !!\n");
}

/* 21-31 Reserved */
