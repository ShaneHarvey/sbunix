#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv) {
    int i = 1, newline = 1;

    /* -n means no trailing newline */
    if(argc > 1 && strcmp("-n", argv[1]) == 0) {
        i++;
        newline = 0;
    }

    for( ; i < argc; i++)
        write(STDOUT_FILENO, argv[i], strlen(argv[i]));

    if(newline)
        write(STDOUT_FILENO, "\n", 1);
    return EXIT_SUCCESS;
}
