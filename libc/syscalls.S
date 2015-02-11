/* Copyright (C) 2001, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

.section .text

# function arguments  rdi, rsi, rdx, rcx, r8, r9
# function returns    rax, rdx

# syscall number      rax
# Arguments           rdi, rsi, rdx, r10, r8, r9.
# syscall return      rax

.global syscall_0
syscall_0:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	syscall
	cmpq $-4095, %rax	/* Check %rax for error.  */
    jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
    ret			/* Return to caller.  */

.global syscall_1
syscall_1:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	syscall			/* Do the system call.  */
	cmpq $-4095, %rax	/* Check %rax for error.  */
    jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
    ret			/* Return to caller.  */

.global syscall_2
syscall_2:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	syscall			/* Do the system call.  */
	cmpq $-4095, %rax	/* Check %rax for error.  */
    jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
    ret			/* Return to caller.  */

.global syscall_3
syscall_3:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	movq %rcx, %rdx
	syscall			/* Do the system call.  */
	cmpq $-4095, %rax	/* Check %rax for error.  */
    jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
    ret			/* Return to caller.  */

.global syscall_4
syscall_4:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	movq %rcx, %rdx
	movq %r8, %r10
	syscall			/* Do the system call.  */
	cmpq $-4095, %rax	/* Check %rax for error.  */
    jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
    ret			/* Return to caller.  */

.global syscall_5
syscall_5:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	movq %rcx, %rdx
	movq %r8, %r10
	movq %r9, %r8
	syscall			/* Do the system call.  */
	cmpq $-4095, %rax	/* Check %rax for error.  */
    jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
    ret			/* Return to caller.  */

.global syscall_6
syscall_6:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	movq %rcx, %rdx
	movq %r8, %r10
	movq %r9, %r8
	movq 8(%rsp),%r9	/* arg6 is on the stack.  */
	syscall			/* Do the system call.  */
	cmpq $-4095, %rax	/* Check %rax for error.  */
    jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
    ret			/* Return to caller.  */

.global errno
SYSCALL_ERROR_LABEL:
    neg %eax
    movl %eax, (errno)
    or $-1, %rax
    ret