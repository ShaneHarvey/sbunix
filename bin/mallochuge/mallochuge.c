#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void usage(char *prgname) {
    exit(1);
}

int test_huge_alloc(void) {
    size_t terabyte = (size_t)(1L<<42);
    char *limit;

    /* Test 1 terabyte malloc */
    char *x = malloc(terabyte);
    if(x == NULL) {
        printf("Error: %s\n", strerror(errno));
        return 1;
    }
    printf("malloc'd (1 Terabyte), starts at %p\n", (void*)x);
    printf("Attempting to write to every at page...\n");

    /* Now write to all 1024 GB, :D */
    limit = x + terabyte - 0x1000;
    for(;x < limit; x += 0x1000) {
        *x = 'A';
    }
    return 0;
}

int main(int argc, char *argv[], char *envp[]) {
    return test_huge_alloc();
}