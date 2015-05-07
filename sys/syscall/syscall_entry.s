# To manually save the current user stack
.global syscall_user_rsp

# To manually restore the current task's kernel stack
.global syscall_kernel_rsp

# The C syscall dispather (sys/syscall/syscall_dispatch.c)
.global syscall_dispatch

#
# Entry point for the syscall instruction.
#   1. Save user stack
#   2. Restore kernel stack
#   3. Call syscall_dispatch to handler the syscall
#   4. Return to user
#
# Register setup:
# rax       system call number
# rdi       arg1
# rcx       return address for syscall/sysret   (arg4 for C code)
# rsi       arg2
# rdx       arg3
# r10       arg4                                (moved to rcx for C code)
# r8        arg5
# r9        arg6
# r11       eflags for syscall/sysret, temporary for C
# r12-r15   rbp,rbx saved by C code, not touched.
#
.global syscall_entry
syscall_entry:
    movq %rsp, syscall_user_rsp     # save user stack
    movq (syscall_kernel_rsp), %rsp # restore task's kernel stack
    pushq (syscall_user_rsp)        # save user rsp on kernel stack
    pushq %r11                      # save user RFLAGS
    pushq %rcx                      # save user RIP onto kern stack
    movq %r10, %rcx                 # switch syscall convention to SYSV C convention

# Save user regs, except rax, rcx, and r11
    pushq %rbx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rbp

    pushq %rax                      # 7th arg must be on stack for SYSV C
    call syscall_dispatch
    jmp restore_and_sysret

# Child returns here from fork, we need to store 0 in rax
.global child_ret_from_fork
child_ret_from_fork:
    xorq %rax, %rax                 # Child returns 0

restore_and_sysret:
    addq $0x8, %rsp                 # pop 7th arg to syscall_dispatch
# Restore user regs, except rax, rcx, and r11
    popq %rbp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rbx

    # Prepare for sysretq
    popq %rcx                       # pop user return addr off kern stack
    popq %r11                       # pop user RFLAGS
    popq %rsp                       # restore user stack
    sysretq
