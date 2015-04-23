#include <sbunix/interrupt/pit.h>
#include <sbunix/interrupt/idt.h>
#include <sbunix/interrupt/pic8259.h>
#include <sbunix/writec.h>
#include <sbunix/asm.h>

/* Programmable Interrupt Timer */

uint64_t system_time  = 0;
uint32_t time_counter = 1;
uint32_t time_reset   = 18;


void ISR_HANDLER(32) {
    time_counter++;
    if(time_counter == time_reset) {
        time_counter = 1;
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
    uint16_t reload_val;
    if(hz <= 18) {
        reload_val = 0; /* 0 reload value for PIT is ~18 HZ */
        time_reset = 18;
    } else {
        reload_val = (uint16_t)(1193180/hz);
        time_reset = hz;
    }
    time_reset = reload_val? hz : 18;
    time_counter = 0;
    cli();
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)reload_val);        /* low byte of reload */
    outb(0x40, (uint8_t)(reload_val >> 8)); /* high byte of reload */
    sti();
}
