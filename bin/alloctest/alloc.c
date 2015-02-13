#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
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
            printf("malloc: %s\n", strerror(errno));
            return 1;
        }
        strncpy(cp, str, 100 * sizeof(char));
        printf("%s\n", cp);
        free(cp);

        ip = malloc(2040);
        if(!ip) {
            printf("malloc: %s\n", strerror(errno));
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
int malloctest(int malloc_num, size_t malloc_size) {
    void **ptrs;
    int i;
    ptrs = malloc((size_t)malloc_num * sizeof(void *));
    if(!ptrs){
        printf("malloc failed: %s\n", strerror(errno));
        return 1;
    }
    for(i = 0; i < malloc_num; i++) {
        ptrs[i] = malloc(malloc_size);
        if(!ptrs[i]) {
            printf("malloc failed: %s\n", strerror(errno));
            return 1;
        }
    }
    for(i = 0; i < malloc_num; i++) {
        free(ptrs[i]);
    }
    free(ptrs);
    return 0;
}

/**
* @alloc_num: Number of times to realloc
* @alloc_size: Starting size of the original malloc
*/
int realloctest(size_t realloc_size) {
    char *ptr = malloc(realloc_size);
    if(!ptr) {
        printf("%s\n", strerror(errno));
        return 1;
    }
    while(realloc_size > 500) {
        ptr = realloc(ptr, realloc_size);
        if(!ptr) {
            printf("realloc failed %s\n", strerror(errno));
            return 1;
        }
        realloc_size -= 500;
    }
    free(ptr);
    return 0;
}


int main(int argc, char *argv[], char *envp[]) {
    int num_mallocs;
    size_t alloc_size;
    if(argc == 2) {
        alloc_size = (size_t) atoi(argv[1]);
        printf("realloctest(%d)\n", (int)alloc_size);
        return realloctest(alloc_size);
    } else if(argc == 3) {
        num_mallocs = atoi(argv[1]);
        alloc_size = (size_t) atoi(argv[2]);
        printf("malloctest(%d, %d)\n", num_mallocs, (int)alloc_size);
        return malloctest(num_mallocs, alloc_size);
    }

    printf("Usage: %s num_mallocs malloc_size\n", argv[0]);
    printf("Usage: %s realloc_size\n", argv[0]);
    return 1;
}