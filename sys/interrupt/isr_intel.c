#include <sys/idt.h>
#include <sys/sbunix.h>

/**
* This file is for Interrupt Service Routine Handlers for the Reserved
* Intel Interrupts (0 - 31).
*/

/**
* Divide by Zero Exception Handler
*/
void ISR_HANDLER(0) {
    printf("!! DIVIDE BY ZERO !!\n");
    while(1) {
        __asm__ __volatile__ ("hlt;");
    }
}
