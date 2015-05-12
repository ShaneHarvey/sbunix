#include <sbunix/sbunix.h>
#include <sbunix/serial.h>

/**
 * This code is taken from http://wiki.osdev.org/Serial_Ports#Example_Code
 */


void serial_init(void) {
    outb(PORT + 1, 0x00);    // Disable all interrupts
    outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00);    //                  (hi byte)
    outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int is_transmit_empty(void) {
    return inb(PORT + 5) & 0x20;
}

void serial_write(uint8_t a) {
    while (is_transmit_empty() == 0);/* nothing */;

    outb(PORT, a);
}

int serial_received(void) {
    return inb(PORT + 5) & 1;
}

uint8_t serial_read(void) {
    while (serial_received() == 0);/* nothing */;

    return inb(PORT);
}
