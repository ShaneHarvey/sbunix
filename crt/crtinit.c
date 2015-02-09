char **__environ = 0;

void _init_sblibc(int argc, char **argv, char **envp) {
    /* initialize __environ which is used by getenv(3)/setenv(3) */
    __environ = envp;
}

void _init(int argc, char **argv, char **envp) {
    /* Run global constructors in the .ctors section */
}