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
        "xorq %%rbp, %%rbp;"            /* ABI: zero rbp */
        "popq %%rdi;"                   /* set argc */
        "movq %%rsp, %%rsi;"            /* set argv */
        "leaq 8(%%rsi,%%rdi,8), %%rdx;" /* Set envp = argv + 8 * argc + 8 */
        "pushq %%rbp;"
        "pushq %%rbp;"
        "andq $-16, %%rsp;"             /* ABI: align stack to 16 bytes */
        "call _init_sblibc;"
        "call _init;"
        "call main;"                    /* run main program */
        "movq %%rax, %%rdi;"            /* exit with return code */
        "call exit;"
        "hlt;":::
    );
}
