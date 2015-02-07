.section .text

.global _start
_start:
    # Set up the stack?
    movq %rsp, %rbp

    # Set argc
    movq 0(%rsp), %rdi
    # Set argv
    movq %rsp, %rsi
    addq $8, %rsi
    # Set envp = argv + 8 * argc + 8
    leaq 8(%rsi,%rdi,8), %rdx

    # Run program
    call main

    # Exit with return code
    movq %rax, %rdi
    call exit
