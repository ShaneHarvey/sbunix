#ifndef _SBUNIX_GDT_H
#define _SBUNIX_GDT_H

#include <sys/defs.h>

#define _KERNEL_CS	0x8
#define _KERNEL_DS	0x10
#define _USER_CS	0x18
#define _USER_DS	0x20

/* See http://forum.osdev.org/viewtopic.php?t=13678 for explanation */
struct tss_t {
	uint32_t reserved;
	uint64_t rsp0;  /* Stores val wanted in RSP when entering kernel from ring3 */
	uint32_t unused[11];
}__attribute__((packed));
extern struct tss_t tss;

extern uint64_t gdt[];

void reload_gdt();
void setup_tss();

#endif
