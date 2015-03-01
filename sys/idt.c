#include <sys/idt.h>

/* An Entry in the Interrupt Descriptor Table(IDT). 12 bytes for x64 */
struct idt_entry {
    uint64_t offset_lo:16;   /* Offset(0...15) bits 0-15 */
    uint64_t segment_sel:16; /* Segment Selector for dest code segment 16-31 */
    uint64_t ist:2;          /* Interrupt Stack Table 32-33 */
    uint64_t zero1:6;        /* Zero 34-39 */
    uint64_t type:4;         /* Type 40-43 */
    uint64_t zero2:1;        /* Zero 44 */
    uint64_t dpl:2;          /* Discriptor Priv Level 45-46 */
    uint64_t p:1;            /* Segement Present Flag 47 */
    uint64_t offset_med:16;  /* Offset(63...32) bits 48-63 */
    uint64_t offset_hi:32;   /* Offset(31...16) bits 64-95 */
    uint64_t reserved:32;    /* Reserved 96-127 */
}__attribute__((packed));