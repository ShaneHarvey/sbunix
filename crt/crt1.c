#include <stdlib.h>
#include <unistd.h>

char **__environ = NULL;

void _start(void) {
    int argc;
    char **argv, **envp;

    __asm__ volatile (
        "movl 8(%%rsp), %0;"
        "movq %%rsp, %1;"
        : "=r"(argc), "=r"(argv)
    );
    argv += 2;
    /* set environment pointer */
    __environ = envp = argv + argc + 1;

    exit(main(argc, argv, envp));
}
