#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {
    char str[] = "hello world!";
    char *cp;
    int *ip;
    int i;

    /* Each loop will require 1 call to sbrk(4096) */
    for(i = 0; i < 2; i++){
        cp = malloc(2040 * sizeof(char));
        if(!cp) {
            printf("malloc 1 Failed!\n");
            return 1;
        }

        strncpy(cp, str, 100 * sizeof(char));
        printf("%s\n", cp);

        ip = malloc(2040);
        if(!ip) {
            printf("malloc 2 Failed!\n");
            return 1;
        }
    }
    return 0;
}