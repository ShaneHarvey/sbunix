#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {
    char str[] = "hello world!";
    char *cp;
    int *ip;

    cp = malloc(100 * sizeof(char));
    if(!cp) {
        printf("malloc 1 Failed!\n");
        return 1;
    }

    ip = malloc(100 * sizeof(int));
    if(!ip) {
        printf("malloc 2 Failed!\n");
        return 1;
    }

    strncpy(cp, str, 100 * sizeof(char));
    printf("%s\n", cp);
    return 0;
}