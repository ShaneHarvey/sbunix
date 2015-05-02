
int main(int argc, char **argv, char **envp) {
    __asm__ __volatile__(
        "label:nop;nop;nop;"
        "jmp label;"
    );
    return 1;
}
