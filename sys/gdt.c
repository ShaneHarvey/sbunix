#include <sys/gdt.h>

/* adapted from Chris Stones, shovelos */

#define MAX_GDT 32

#define GDT_CS        (0x00180000000000)  /*** code segment descriptor ***/
#define GDT_DS        (0x00100000000000)  /*** data segment descriptor ***/

#define C             (0x00040000000000)  /*** conforming ***/
#define DPL0          (0x00000000000000)  /*** descriptor privilege level 0 ***/
#define DPL1          (0x00200000000000)  /*** descriptor privilege level 1 ***/
#define DPL2          (0x00400000000000)  /*** descriptor privilege level 2 ***/
#define DPL3          (0x00600000000000)  /*** descriptor privilege level 3 ***/
#define P             (0x00800000000000)  /*** present ***/
#define L             (0x20000000000000)  /*** long mode ***/
#define D             (0x40000000000000)  /*** default op size ***/
#define W             (0x00020000000000)  /*** writable data segment ***/

struct sys_segment_descriptor {
	uint64_t sd_lolimit:16;/* segment extent (lsb) */
	uint64_t sd_lobase:24; /* segment base address (lsb) */
	uint64_t sd_type:5;    /* segment type */
	uint64_t sd_dpl:2;     /* segment descriptor priority level */
	uint64_t sd_p:1;       /* segment descriptor present */
	uint64_t sd_hilimit:4; /* segment extent (msb) */
	uint64_t sd_xx1:3;     /* avl, long and def32 (not used) */
	uint64_t sd_gran:1;    /* limit granularity (byte/page) */
	uint64_t sd_hibase:40; /* segment base address (msb) */
	uint64_t sd_xx2:8;     /* reserved */
	uint64_t sd_zero:5;    /* must be zero */
	uint64_t sd_xx3:19;    /* reserved */
}__attribute__((packed));

uint64_t gdt[MAX_GDT] = {
	0,                      /*** NULL descriptor ***/
	GDT_CS | P | DPL0 | L,  /*** kernel code segment descriptor ***/
	GDT_DS | P | W | DPL0,  /*** kernel data segment descriptor ***/
	GDT_CS | P | DPL3 | L,  /*** user code segment descriptor ***/
	GDT_DS | P | W | DPL3,  /*** user data segment descriptor ***/
	0, 0,                   /*** TSS ***/
};

struct gdtr_t {
	uint16_t size;
	uint64_t addr;
}__attribute__((packed));

static struct gdtr_t gdtr = {
	(uint16_t)sizeof(gdt),
	(uint64_t)gdt,
};

extern void _x86_64_asm_lgdt(struct gdtr_t* gdtr, uint64_t cs_idx, uint64_t ds_idx); /* gdt.s */

void reload_gdt() {
	_x86_64_asm_lgdt(&gdtr, 8, 16);
}


/** From OS Dev:
* The actual loading of the TSS must take place in protected mode and after
* the GDT has been loaded. The loading is simple as:
*   mov ax, 0x??  ;The descriptor of the TSS in the GDT(e.g. 0x28 if the sixths entry in your GDT describes your TSS)
*   ltr ax        ;The actual load
*
* Also see: http://wiki.osdev.org/Descriptor
*/
void setup_tss() {
	struct sys_segment_descriptor* sd = (struct sys_segment_descriptor*)&gdt[5]; /* 6th&7th entry in GDT */
	sd->sd_lolimit = sizeof(struct tss_t)-1;
	sd->sd_lobase = ((uint64_t)&tss);
	sd->sd_type = 9; /* 80386-TSS, 32 bit */
	sd->sd_dpl = 0;
	sd->sd_p = 1;
	sd->sd_hilimit = 0;
	sd->sd_gran = 0;
	sd->sd_hibase = ((uint64_t)&tss) >> 24;
}
