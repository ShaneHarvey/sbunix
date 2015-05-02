
int main(int argc, char **argv, char **envp) {
    __asm__ __volatile__(
        "label:nop;"
        "testl %eax, %eax;"
        "je label;"
    );
    return 1;
}