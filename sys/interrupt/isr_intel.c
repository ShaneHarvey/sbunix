#include <sbunix/idt.h>
#include <sbunix/sbunix.h>

#define halt_msg(msg) printf(msg); \
    while(1) { \
    __asm__ __volatile__ ("hlt;"); \
    }

/**
* This file is for Interrupt Service Routine Handlers for the Reserved
* Intel Interrupts (0 - 31).
*/

/**
* Divide by Zero Exception Handler (#DE)
*/
void ISR_HANDLER(0) {
    halt_msg("!! DIVIDE BY ZERO !!\n");
}

void ISR_HANDLER(1) {
    halt_msg("!! Debug Exception (#DB) !!\n");
}

void ISR_HANDLER(2) {
    halt_msg("!! NMI Interrupt !!\n");
}

void ISR_HANDLER(3) {
    halt_msg("!! Breakpoint Exception (#BP) !!\n");
}

void ISR_HANDLER(4) {
    halt_msg("!! Overflow Exception (#OF) !!\n");
}

void ISR_HANDLER(5) {
    halt_msg("!! BOUND Range Exceeded Exception (#BR) !!\n");
}

void ISR_HANDLER(6) {
    halt_msg("!! Invalid Opcode Exception (#UD) !!\n");
}

void ISR_HANDLER(7) {
    halt_msg("!! Device Not Available Exception (#NM) !!\n");
}

void ISR_HANDLER(8) {
    halt_msg("!! Double Fault Exception (#DF) !!\n");
}

void ISR_HANDLER(9) {
    halt_msg("!! Coprocessor Segment Overrun !!\n");
}

void ISR_HANDLER(10) {
    halt_msg("!! Invalid TSS Exception (#TS) !!\n");
}

void ISR_HANDLER(11) {
    halt_msg("!! Segment Not Present (#NP) !!\n");
}

void ISR_HANDLER(12) {
    halt_msg("!! Stack Fault Exception (#SS) !!\n");
}

void ISR_HANDLER(13) {
    halt_msg("!! General Protection Exception (#GP) !!\n");
}

void ISR_HANDLER(14) {
    halt_msg("!! Page-Fault Exception (#PF) !!\n");
}

/* 15 Reserved */

void ISR_HANDLER(16) {
    halt_msg("!! x87 FPU Floating-Point Erro (#MF) !!\n");
}

void ISR_HANDLER(17) {
    halt_msg("!! Alignment Check Exception (#AC) !!\n");
}

void ISR_HANDLER(18) {
    halt_msg("!! Machine-Check Exception (#MC) !!\n");
}

void ISR_HANDLER(19) {
    halt_msg("!! SIMD Floating-Point Exception (#XM) !!\n");
}

void ISR_HANDLER(20) {
    halt_msg("!! Virtualization Exception (#VE) !!\n");
}

/* 21-31 Reserved */
