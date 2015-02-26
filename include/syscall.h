#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>

/* function arguments  rdi, rsi, rdx, rcx, r8, r9
 * function returns    rax, rdx

 * syscall number      rax
 * arguments           rdi, rsi, rdx, r10, r8, r9.
 * syscall return      rax
 */
uint64_t syscall_0(uint64_t n);

uint64_t syscall_1(uint64_t n, uint64_t a1);

uint64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2);

uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3);

uint64_t syscall_4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3,
		uint64_t a4);

uint64_t syscall_5(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3,
		uint64_t a4, uint64_t a5);

uint64_t syscall_6(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3,
		uint64_t a4, uint64_t a5, uint64_t a6);

#endif
