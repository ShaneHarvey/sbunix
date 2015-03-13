#ifndef _SBUNIX_PORTS_H
#define _SBUNIX_PORTS_H

#include <sys/defs.h>

/* http://wiki.osdev.org/Inline_Assembly/Examples */

/**
* Send 1 byte val out on specified port.
*/
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ (
        "outb %0, %1"
        :
        : "a"(val), "Nd"(port)
    );
}

/**
* Receive 1 byte off the the specified port.
*/
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ (
        "inb %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}

/**
* Write to an unused port (just to kill time?)
*/
static inline void io_wait(void) {
    outb(0x80, 0);
}

/* Could also define in[wl], out[wl] for 16/32 bits */
#endif
