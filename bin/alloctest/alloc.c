#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>

int simpletest(void) {
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
        free(cp);

        ip = malloc(2040);
        if(!ip) {
            printf("malloc 2 Failed!\n");
            return 1;
        }
        free(ip);
    }
    return 0;
}

/**
* @alloc_num: Number of times to malloc
* @alloc_size: Size of each malloc
*/
int malloctest(int alloc_num, size_t alloc_size) {
    char *ptrs[alloc_num];
    int i;

    for(i = 0; i < alloc_num; i++) {
        ptrs[i] = malloc(alloc_size);
        if(!ptrs[i]) {
            /*printf(strerror(errno));*/
            error("malloc failed!");
        }
    }
    for(i = 0; i < alloc_num; i++) {
        free(ptrs[i]);
    }
    return 0;
}


int main(int argc, char *argv[], char *envp[]) {
    if(argc != 3) {
        printf("Usage: %s num_allocs alloc_size\n", argv[0]);
        return 1;
    }
    printf("%d\n%d\n", atoi(argv[1]), atoi(argv[2]));
    return malloctest(atoi(argv[1]), (size_t)atoi(argv[2]));
}