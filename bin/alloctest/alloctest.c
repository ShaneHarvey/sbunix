#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void usage(char *prgname) {
    exit(1);
}

int test_huge_alloc(void) {
    size_t terabyte = (size_t)(1L<<42);

    /* Test 1 terabyte malloc */
    char *x = malloc(terabyte);
    if(x == NULL) {
        printf("Error: %s\n", strerror(errno));
        return 1;
    }
    printf("malloc'd (1 Terabyte), starts at %p\n", (void*)x);

    /* Now write to all 1024 GB, :D */
    for(; (size_t)x < terabyte - 0x1000; x += 0x1000) {
        *x = 'A';
    }
    return 0;
}

int main(int argc, char *argv[], char *envp[]) {
    return test_huge_alloc();
}