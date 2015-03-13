#include <sbunix/sbunix.h>
#include <sbunix/idt.h>
#include <sbunix/asm.h>

/**
* This file is for Interrupt Service Routine Handlers for the Reserved
* Intel Interrupts (0 - 31).
*/

/**
* Divide by Zero Exception Handler (#DE)
*/
void ISR_HANDLER(0) {
    halt_loop("!! DIVIDE BY ZERO !!\n");
}

void ISR_HANDLER(1) {
    halt_loop("!! Debug Exception (#DB) !!\n");
}

void ISR_HANDLER(2) {
    halt_loop("!! NMI Interrupt !!\n");
}

void ISR_HANDLER(3) {
    halt_loop("!! Breakpoint Exception (#BP) !!\n");
}

void ISR_HANDLER(4) {
    halt_loop("!! Overflow Exception (#OF) !!\n");
}

void ISR_HANDLER(5) {
    halt_loop("!! BOUND Range Exceeded Exception (#BR) !!\n");
}

void ISR_HANDLER(6) {
    halt_loop("!! Invalid Opcode Exception (#UD) !!\n");
}

void ISR_HANDLER(7) {
    halt_loop("!! Device Not Available Exception (#NM) !!\n");
}

void ISR_HANDLER(8) {
    halt_loop("!! Double Fault Exception (#DF) !!\n");
}

void ISR_HANDLER(9) {
    halt_loop("!! Coprocessor Segment Overrun !!\n");
}

void ISR_HANDLER(10) {
    halt_loop("!! Invalid TSS Exception (#TS) !!\n");
}

void ISR_HANDLER(11) {
    halt_loop("!! Segment Not Present (#NP) !!\n");
}

void ISR_HANDLER(12) {
    halt_loop("!! Stack Fault Exception (#SS) !!\n");
}

void ISR_HANDLER(13) {
    halt_loop("!! General Protection Exception (#GP) !!\n");
}

void ISR_HANDLER(14) {
    halt_loop("!! Page-Fault Exception (#PF) !!\n");
}

/* 15 Reserved */

void ISR_HANDLER(16) {
    halt_loop("!! x87 FPU Floating-Point Erro (#MF) !!\n");
}

void ISR_HANDLER(17) {
    halt_loop("!! Alignment Check Exception (#AC) !!\n");
}

void ISR_HANDLER(18) {
    halt_loop("!! Machine-Check Exception (#MC) !!\n");
}

void ISR_HANDLER(19) {
    halt_loop("!! SIMD Floating-Point Exception (#XM) !!\n");
}

void ISR_HANDLER(20) {
    halt_loop("!! Virtualization Exception (#VE) !!\n");
}

/* 21-31 Reserved */
