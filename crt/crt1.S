.section .text

.global _start
_start:
    # Set up the stack?
    movq %rsp, %rbp

    # Set argc
    movq 8(%rsp), %rdi
    # Set argv
    movq %rsp, %rsi
    addq $16, %rsi
    # Set envp
    movq %rsp, %rdx
    addq $32, %rdx

    # Run program
    call main

    # Exit with return code
    movq %rax, %rdi
    call exit
