#ifndef _IDT_H
#define _IDT_H

#include <sys/defs.h>

#define ISR_HANDLER(vector) _isr_hanlder_ # vector()

/**
* An entry in the Interrupt Descriptor Table(IDT). 16 bytes for x86_64.
* The pointer is split across the 3 offset fields.
* See Figure 6-7. 64-Bit IDT Gate Descriptors, of the Intel 64 manual
*/
struct idt_t {
    uint16_t offset_lo;   /* Offset(0...15), bits 0-15 */
    uint16_t selector;    /* Segment Selector for dest code segment, 16-31 */
    uint16_t ist:3;       /* Interrupt Stack Table (also zero), 32-34 */
    uint16_t zero1:5;     /* Zero, bits 35-39 */
    uint16_t type:4;      /* Type, bits 40-43 */
    uint16_t zero2:1;     /* Zero, bit 44 */
    uint16_t dpl:2;       /* Discriptor Priv Level (ring 0 or 3), bits 45-46 */
    uint16_t p:1;         /* Segement Present Flag, bit 47 */
    uint16_t offset_med;  /* Offset(63...32), bits 48-63 */
    uint32_t offset_hi;   /* Offset(31...16), bits 64-95 */
    uint32_t reserved;    /* Reserved, bits 96-127 */
}__attribute__((packed));

void load_idt(void);

#endif
