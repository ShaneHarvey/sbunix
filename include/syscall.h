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
int64_t syscall_0(int64_t n);

int64_t syscall_1(int64_t n, int64_t a1);

int64_t syscall_2(int64_t n, int64_t a1, int64_t a2);

int64_t syscall_3(int64_t n, int64_t a1, int64_t a2, int64_t a3);

int64_t syscall_4(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4);

int64_t syscall_5(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
				  int64_t a5);

int64_t syscall_6(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
				  int64_t a5, int64_t a6);

#endif
