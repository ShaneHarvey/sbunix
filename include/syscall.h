#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>


#define SYSCALL_ERROR_CHECK(rv) do{if(rv < 0 && rv > -4095){errno = -rv; rv = -1;}} while(0)

/* function arguments  rdi, rsi, rdx, rcx, r8, r9
 * function returns    rax, rdx

 * syscall number      rax
 * arguments           rdi, rsi, rdx, r10, r8, r9.
 * syscall return      rax
 */
static __inline int64_t syscall_0(int64_t n) {
	int64_t rv;
	__asm__ volatile (
		"movq %1, %%rax;"
		"syscall;"
		"movq %%rax, %0;"
		: "=g"(rv)  /* output */
		:"g"(n)     /* input */
		: "%rax"    /* clobbered registers */
	);
	return rv;
}

static __inline int64_t syscall_1(int64_t n, int64_t a1) {
	int64_t rv;
	__asm__ volatile (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"syscall;"
		"movq %%rax, %0;"
		: "=g"(rv)
		:"g"(n), "g"(a1)
		: "%rax", "%rdi"
	);
	return rv;
}

static __inline int64_t syscall_2(int64_t n, int64_t a1, int64_t a2) {
	int64_t rv;
	__asm__ volatile (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"movq %3, %%rsi;"
		"syscall;"
		"movq %%rax, %0;"
		: "=g"(rv)
		:"g"(n), "g"(a1), "g"(a2)
		: "%rax", "%rdi", "%rsi"
	);
	return rv;
}

static __inline int64_t syscall_3(int64_t n, int64_t a1, int64_t a2,
		int64_t a3) {
	int64_t rv;
	__asm__ volatile (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"movq %3, %%rsi;"
		"movq %4, %%rdx;"
		"syscall;"
		"movq %%rax, %0;"
		: "=g"(rv)
		:"g"(n), "g"(a1), "g"(a2), "g"(a3)
		: "%rax", "%rdi", "%rsi", "%rdx"
	);
	return rv;
}

static __inline int64_t syscall_4(int64_t n, int64_t a1, int64_t a2,
		int64_t a3, int64_t a4) {
	int64_t rv;
	__asm__ volatile (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"movq %3, %%rsi;"
		"movq %4, %%rdx;"
		"movq %5, %%r10;"
		"syscall;"
		"movq %%rax, %0;"
		: "=g"(rv)
		:"g"(n), "g"(a1), "g"(a2), "g"(a3), "g"(a4)
		: "%rax", "%rdi", "%rsi", "%rdx", "%r10"
	);
	return rv;
}

static __inline int64_t syscall_5(int64_t n, int64_t a1, int64_t a2,
		int64_t a3, int64_t a4, int64_t a5) {
	int64_t rv;
	__asm__ volatile (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"movq %3, %%rsi;"
		"movq %4, %%rdx;"
		"movq %5, %%r10;"
		"movq %6, %%r8;"
		"syscall;"
		"movq %%rax, %0;"
		: "=g"(rv)
		:"g"(n), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5)
		: "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8"
	);
	return rv;
}

static __inline int64_t syscall_6(int64_t n, int64_t a1, int64_t a2,
		int64_t a3, int64_t a4, int64_t a5,
		int64_t a6) {
	int64_t rv;
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
		: "=g"(rv)
		:"g"(n), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5), "g"(a6)
		: "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "%r9"
	);
	return rv;
}

#endif
