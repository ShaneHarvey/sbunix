char **__environ = 0;
__thread int errno;

void _init_sblibc(int argc, char **argv, char **envp) {
    /* initialize __environ which is used by getenv(3)/setenv(3) */
    __environ = envp;
}

void _init(int argc, char **argv, char **envp) {
    /* Run global constructors in the .ctors section */
}

void _start(void) {
    __asm__ volatile (
        /* Set up the stack? */
        "movq %%rsp, %%rbp;"
        /* Load argc */
        "movq 0(%%rsp), %%rdi;"
        /* Load argv */
        "movq %%rsp, %%rsi;"
        "addq $8, %%rsi;"
        /* Set envp = argv + 8 * argc + 8 */
        "leaq 8(%%rsi,%%rdi,8), %%rdx;"
        /* Call initialization routines */
        "call _init_sblibc;"
        "call _init;"
        /* Run program */
        "call main;"
        /* Exit with return code */
        "movq %%rax, %%rdi;"
        "call exit;":::
    );
}
