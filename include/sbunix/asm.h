#ifndef _SBUNIX_ASM_H
#define _SBUNIX_ASM_H

/* Header for common assembly routines. */

#define PUSHQALL \
        "pushq %%rax;" \
        "pushq %%rbx;" \
        "pushq %%rcx;" \
        "pushq %%rdx;" \
        "pushq %%rsi;" \
        "pushq %%rdi;" \
        "pushq %%r8;"  \
        "pushq %%r9;"  \
        "pushq %%r10;" \
        "pushq %%r11;" \
        "pushq %%r12;" \
        "pushq %%r13;" \
        "pushq %%r14;" \
        "pushq %%r15;" \
        "pushq %%rbp;"


#define POPQALL \
        "popq %%rbp;" \
        "popq %%r15;" \
        "popq %%r14;" \
        "popq %%r13;" \
        "popq %%r12;" \
        "popq %%r11;" \
        "popq %%r10;" \
        "popq %%r9;"  \
        "popq %%r8;"  \
        "popq %%rdi;" \
        "popq %%rsi;" \
        "popq %%rdx;" \
        "popq %%rcx;" \
        "popq %%rbx;" \
        "popq %%rax;"

#define halt_loop(fmt, ...) \
            do { \
                printk(fmt, ##__VA_ARGS__); \
                while(1) { \
                    __asm__ __volatile__ ("hlt;"); \
                } \
            } while(0)

/**
 * Magic breakpoint instruction for bochs
 */
static inline void bochs_breakpoint(void) {
    __asm__ __volatile__ ("xchgw %bx, %bx");
}

/**
 * Read the current value of the stack pointer
 */
static inline uint64_t read_rsp(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(ret));
    return ret;
}


/* http://wiki.osdev.org/Inline_Assembly/Examples */
/**
 * Send 1 byte val out on specified port.
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

/**
 * Receive 1 byte off the the specified port.
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/**
 * Disable interrupts
 */
static inline void cli(void) {
    __asm__ __volatile__ ("cli");
}

/**
 * Enable interrupts
 */
static inline void sti(void) {
    __asm__ __volatile__ ("sti");
}


/**
 * Read a 64-bit value from a MSR. The A constraint stands for concatenation
 * of registers EAX and EDX.
 */
static inline uint64_t rdmsr(uint32_t msr_id) {
    uint32_t msr_lo, msr_hi;
    __asm__ __volatile__ ( "rdmsr" : "=a" (msr_lo), "=d" (msr_hi) : "c" (msr_id));
    return (uint64_t) msr_hi <<32 | (uint64_t) msr_lo;
}

/**
 * Write a 64-bit value to a MSR. The A constraint stands for concatenation
 * of registers EAX and EDX.
 */
static inline void wrmsr(uint32_t msr_id, uint64_t msr_val) {
    uint32_t msr_lo, msr_hi;
    msr_lo = (uint32_t)msr_val;
    msr_hi = (uint32_t)(msr_val >> 32);
    __asm__ __volatile__ ("wrmsr" : : "a"(msr_lo), "d"(msr_hi), "c"(msr_id));
}

static inline uint64_t read_cr0(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr0, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr1(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr1, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr2(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr2, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr3(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr3, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr4(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr4, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr8(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr8, %0;":"=r"(ret));
    return ret;
}

static inline void write_cr3(uint64_t pml4e_ptr) {
    __asm__ __volatile__ ("movq %0, %%cr3;"::"r"(pml4e_ptr):"memory");
}

/* Intel/AMD Machine Specific Registers (MSR's) */
#define MSR_EFER    0xC0000080
#define MSR_STAR    0xC0000081
#define MSR_LSTAR   0xC0000082
#define MSR_CSTAR   0xC0000083
#define MSR_SFMASK  0xC0000084

#define MSR_EFER_SCE    0x1

#endif
