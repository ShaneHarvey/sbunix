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

    # Call to initialize globals used by sblibc
    call _init_sblibc

    # Call _init to run constructors
    call _init

    # Run program
    call main

    # Exit with return code
    movq %rax, %rdi
    call exit
