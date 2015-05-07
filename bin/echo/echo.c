#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv) {
    int i = 1, newline = 1, first = 1;;

    if(argc > 1) {
        /* -n means no trailing newline */
        if(strcmp("-n", argv[1]) == 0) {
            i++;
            newline = 0;
        } else if(strcmp("--help", argv[1]) == 0) {
            printf("echo: [-n|--help] STRING...\n");
            return EXIT_SUCCESS;
        }
    }

    for( ; i < argc; i++) {
        if(!first)
            write(STDOUT_FILENO, " ", 1);
        first = 0;
        write(STDOUT_FILENO, argv[i], strlen(argv[i]));
    }

    if(newline)
        write(STDOUT_FILENO, "\n", 1);
    return EXIT_SUCCESS;
}
