#include <sbunix/interrupt/pit.h>
#include <sbunix/interrupt/idt.h>
#include <sbunix/interrupt/pic8259.h>
#include <sbunix/writec.h>
#include <sbunix/asm.h>

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

/**
 * Initialize the PIT to generate interrupts at the specified interval
 * @hz: The frequency in hertz to generate timer interrupts
 */
void pit_set_freq(unsigned int hz) {
    uint16_t reload_val = 0;//(uint16_t)(1193180 / hz);
    time_reset = hz;
    time_counter = 0;
    cli();
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)reload_val);        /* low byte of reload */
    outb(0x40, (uint8_t)(reload_val >> 8)); /* high byte of reload */
    sti();
}
