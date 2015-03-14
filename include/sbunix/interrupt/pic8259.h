#ifndef _SBUNIX_INTERRUPT_PIC8259_H
#define _SBUNIX_INTERRUPT_PIC8259_H

#include <sys/defs.h>

/* Master 8259 PIC Ports */
#define PIC_M_CMD   0x20
#define PIC_M_DATA  0x21

/* Slave 8259 PIC Ports */
#define PIC_S_CMD   0xA0
#define PIC_S_DATA  0xA1

/* PIC Setup */
void PIC_protected_mode(void);
void PIC_remap(uint8_t offset1, uint8_t offset2);

/* Interrupt Routines */
void PIC_sendEOI(uint8_t irq);

void IRQ_set_mask(uint8_t IRQline);
void IRQ_clear_mask(uint8_t IRQline);

uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);


#endif
