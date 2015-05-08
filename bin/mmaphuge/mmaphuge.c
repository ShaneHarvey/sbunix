#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#define handle_error(msg) \
    do { printf(msg ": %s\n", strerror(errno)); \
         exit(EXIT_FAILURE);                               \
    } while (0)

int main(int argc, char *argv[], char *envp[]) {
    int err;
    char *addr, *limit;
    size_t terabyte = (size_t)(1L<<42);

    printf("mmap'ing 1TB of anonymous memory.. ");
    addr = mmap(NULL, terabyte, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    if(addr == MAP_FAILED)
        handle_error("mmap");
    printf("mapped 1TB at %p\n", addr);
    printf("Attempting to write to every at page...\n");

    /* Now write to all 1024 GB, :D */
    limit = addr + terabyte - 0x1000;
    for(;addr < limit; addr += 0x1000) {
        *addr = 'A';
    }

    printf("\nmunmap'ing file...\n");
    err = munmap(addr, terabyte);
    if(err < 0)
        handle_error("munmap");

    return 0;
}
