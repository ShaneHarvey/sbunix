#ifndef _SBUNIX_ASM_H
#define _SBUNIX_ASM_H

/* Header for common assembly routines. */

#define halt_loop(fmt, ...) \
            do { \
                printf(fmt, ##__VA_ARGS__); \
                while(1) { \
                    __asm__ __volatile__ ("hlt;"); \
                } \
            } while(0)

/**
 * Read the current value of the stack pointer
 */
static inline uint64_t read_rsp(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(ret));
    return ret;
}

/**
 * Read a 64-bit value from a MSR. The A constraint stands for concatenation
 * of registers EAX and EDX.
 */
static inline uint64_t rdmsr(uint32_t msr_id) {
    uint64_t msr_value;
    __asm__ __volatile__ ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
    return msr_value;
}

/**
 * Write a 64-bit value to a MSR. The A constraint stands for concatenation
 * of registers EAX and EDX.
 */
static inline void wrmsr(uint32_t msr_id, uint64_t msr_value) {
    __asm__ __volatile__ ( "wrmsr" : : "c" (msr_id), "A" (msr_value) );
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

#endif
