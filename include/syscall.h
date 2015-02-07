#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>
/* n = %rax */
/* %rdi, %rsi, %rdx, %r10, %r8 and %r9. */
static __inline uint64_t syscall_0(uint64_t n) {
    uint64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r"(rv)  /* output */
        :"r"(n)     /* input */
        : "%rax"    /* clobbered registers */
    );
	return rv;
}

static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
    uint64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r"(rv)
        :"r"(n), "r"(a1)
        : "%rax", "%rdi"
    );
    return rv;
}

static __inline uint64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2) {
    uint64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r"(rv)
        :"r"(n), "r"(a1), "r"(a2)
        : "%rax", "%rdi", "%rsi"
    );
    return rv;
}

static __inline uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2,
                                    uint64_t a3) {
    uint64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r"(rv)
        :"r"(n), "r"(a1), "r"(a2), "r"(a3)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );
    return rv;
}

static __inline uint64_t syscall_4(uint64_t n, uint64_t a1, uint64_t a2,
                                    uint64_t a3, uint64_t a4) {
    uint64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "movq %5, %%r10;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r"(rv)
        :"r"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10"
    );
    return rv;
}

static __inline uint64_t syscall_5(uint64_t n, uint64_t a1, uint64_t a2,
                                    uint64_t a3, uint64_t a4, uint64_t a5) {
    uint64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "movq %5, %%r10;"
        "movq %6, %%r8;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r"(rv)
        :"r"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8"
    );
    return rv;
}

static __inline uint64_t syscall_6(uint64_t n, uint64_t a1, uint64_t a2,
                                    uint64_t a3, uint64_t a4, uint64_t a5,
                                    uint64_t a6) {
    uint64_t rv;
    __asm__ volatile (
        "movq %1, %%rax;"
        "movq %2, %%rdi;"
        "movq %3, %%rsi;"
        "movq %4, %%rdx;"
        "movq %5, %%r10;"
        "movq %6, %%r8;"
        "movq %7, %%r9;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r"(rv)
        :"r"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "%r9"
    );
    return rv;
}

#endif
