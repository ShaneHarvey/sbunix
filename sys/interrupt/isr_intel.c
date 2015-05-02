#include <sbunix/sbunix.h>
#include <sbunix/interrupt/idt.h>
#include <sbunix/asm.h>

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
void _isr_handler_13(uint64_t addr, uint64_t errorcode) {
    printk("!! General Protection Exception (#GP) at %p, errorcode 0x%lx !!\n", (void*)addr, errorcode);
    kpanic("!! General Protection Exception (#GP) !!\n");
}

/**
* Page fault handler.
*
* @address:     The faulting address.
* @errorcode:   Placed on stack by processor.
*/
void _isr_handler_14(uint64_t addr, uint64_t errorcode) {
    static char *pf_who[] = { "Kernel ", "User "};
    static char *pf_read[] = { "read ", "write "};
    static char *pf_prot[] = { "non-present", "protection"};
    static char *pf_rsvd[] = { "", "reserved "};
    static char *pf_inst[] = { "", ", instr fetch "};

    printk("!! Page-Fault Exception (#PF) at %p, errorcode 0x%lx !!\n", (void*)addr, errorcode);
    kpanic("!! %s%s%s%s%s !!\n",
           pf_who[(errorcode & PF_USER) == PF_USER],
           pf_rsvd[(errorcode & PF_RSVD) == PF_RSVD],
           pf_read[(errorcode & PF_WRITE) == PF_WRITE],
           pf_inst[(errorcode & PF_INSTR) == PF_INSTR],
           pf_prot[(errorcode & PF_PROT) == PF_PROT]);
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
