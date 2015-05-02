# To manually save the current user stack
.globl syscall_user_rsp

# To manually restore the current task's kernel stack
.globl syscall_kernel_rsp

# The C syscall dispather (sys/syscall/syscall_dispatch.c)
.globl syscall_dispatch

#
# Entry point for the syscall instruction.
#   1. Save user stack
#   2. Restore kernel stack
#   3. Call syscall_dispatch to handler the syscall
#   4. Return to user
#
# Register setup:
# rax  system call number
# rdi       arg0
# rcx       return address for syscall/sysret   (arg3 for C code)
# rsi       arg1
# rdx       arg2
# r10       arg3                                (moved to rcx for C code)
# r8        arg4
# r9        arg5
# r11       eflags for syscall/sysret, temporary for C
# r12-r15   rbp,rbx saved by C code, not touched.
#
.globl syscall_entry
syscall_entry:
    movq %rsp, syscall_user_rsp    # save user stack
    movq syscall_kernel_rsp, %rsp  # restore task's kernel stack
    pushq %rcx                      # push user return addr onto kern stack
    movq %r10, %rcx                 # switch syscall convention to SYSV C convention

    call syscall_dispatch

    popq %rcx                       # pop user return addr off kern stack
    movq %rsp, syscall_kernel_rsp  # reset kernel stack
    movq syscall_user_rsp, %rsp    # restore user stack
    sysret
