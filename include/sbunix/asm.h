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
