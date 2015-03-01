#ifndef _GDT_H
#define _GDT_H

#include <sys/defs.h>

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
