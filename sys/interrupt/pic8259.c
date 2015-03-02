#include <sys/pic8259.h>
#include <sys/ports.h>

#define PIC_EOI		0x20		/* End-of-interrupt command code */

void PIC_sendEOI(uint8_t irq)
{
    if(irq >= 8)
        outb(PIC_S_CMD,PIC_EOI);

    outb(PIC_M_CMD,PIC_EOI);
}

void PIC_protected_mode(void) {
    PIC_remap(0x20, 0x28);
}

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04	/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08	/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C	/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(uint8_t offset1, uint8_t offset2) {
    outb(PIC_M_CMD, ICW1_INIT+ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    outb(PIC_S_CMD, ICW1_INIT+ICW1_ICW4);

    outb(PIC_M_DATA, offset1);            // ICW2: Master PIC vector offset
    outb(PIC_S_DATA, offset2);            // ICW2: Slave PIC vector offset

    outb(PIC_M_DATA, 2);                  /* ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0010) */
    outb(PIC_S_DATA, 2);                  /* ICW3: tell Slave PIC its cascade identity (0000 0010) */

    outb(PIC_M_DATA, ICW4_8086);          /* 8086 mode */
    outb(PIC_S_DATA, ICW4_8086);
}

void IRQ_set_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC_M_DATA;
    } else {
        port = PIC_S_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);
}

void IRQ_clear_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC_M_DATA;
    } else {
        port = PIC_S_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);
}

#define PIC_READ_IRR                0x0A    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0B    /* OCW3 irq service next CMD read */

/* Helper func */
static uint16_t __pic_get_irq_reg(uint8_t ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC_M_CMD, ocw3);
    outb(PIC_S_CMD, ocw3);
    return (inb(PIC_S_CMD) << 8) | inb(PIC_M_CMD);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}
