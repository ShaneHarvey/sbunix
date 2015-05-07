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
.global return_from_fork
.global syscall_entry
syscall_entry:
    movq %rsp, syscall_user_rsp     # save user stack
    movq (syscall_kernel_rsp), %rsp # restore task's kernel stack
    pushq (syscall_user_rsp)        # save user rsp on kernel stack
    pushq %r11                      # save user RFLAGS
    pushq %rcx                      # save user RIP onto kern stack
    movq %r10, %rcx                 # switch syscall convention to SYSV C convention

    pushq %rax                      # 7th arg must be on stack for SYSV C

    call syscall_dispatch
return_from_fork:                   # Child returns here from fork
    addq $0x8, %rsp                 # pop 7th arg

    # Prepare for sysret
    popq %rcx                       # pop user return addr off kern stack
    popq %r11                       # pop user RFLAGS
    popq %rsp                       # restore user stack
    sysretq
