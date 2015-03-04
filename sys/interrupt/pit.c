#include <sys/pit.h>
#include <sys/idt.h>
#include <sys/writec.h>
#include <sys/pic8259.h>

/* Programmable Interrupt Timer */

uint64_t system_time  = 0;
uint32_t time_counter = 0;
uint32_t time_reset   = 0;


void ISR_HANDLER(32) {
    time_counter++;
    if(time_counter == time_reset) {
        time_counter = 0;
        system_time++;
        /* Print Seconds since boot in lower right corner of the console */
        writec_time(system_time);
    }
    /* Acknowledge interrupt */
    PIC_sendEOI(32);
}

int pit_set_freq(double freq) {
    uint16_t reload_val = 0x0000;

    time_reset = 18;
    time_counter = 0;
    /* Setup the PIT to send an INT at 18HZ */
    __asm__ __volatile__ (
        "cli;"
        "movb $0x36, %%al;" /* Init the PIT in Rate Generator mode */
        "out %%al, $0x43;"
        "movw %0, %%ax;"    /* Set the reload value of the PIT */
        "out %%al, $0x40;"  /* send reload lower 8 bits */
        "movb %%ah, %%al;"
        "out %%al, $0x40;"  /* send reload upper 8 bits */
        "sti;"
        :
        : "g"(reload_val)
    );
    return 1;
}
