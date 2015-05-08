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
    int fd, err;
    off_t offset = 0;
    size_t length = 200;
    ssize_t wrote;
    void *addr;
    char *ptr;
    printf("open'ing my src code at /test/mmaptest.c ...\n");

    fd = open("/test/mmaptest.c", O_RDONLY);
    if(fd < 0)
        handle_error("open");

    printf("mmap'ing my src code...\n");
    addr = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, offset);
    if(addr == MAP_FAILED)
        handle_error("mmap");

    printf("Printing my src code...\n");
    wrote = write(STDOUT_FILENO, addr, length);
    if(wrote != length) {
        if(wrote < 0)
            handle_error("write");
        printf("\nwrite: partial write\n");
    }
    printf("Overwriting with A's\n");
    for(ptr = addr; ptr < ((char*)addr) + length; ptr++) {
        *ptr = 'A';
    }
    wrote = write(STDOUT_FILENO, addr, length);
    if(wrote != length) {
        if(wrote < 0)
            handle_error("write");
        printf("\nwrite: partial write\n");
    }

    printf("munmap'ing my src code...\n");
    err = munmap(addr, length);
    if(err < 0)
        handle_error("munmap");

    printf("close'ing file...");
    err = close(fd);
    if(err < 0)
        handle_error("close");

    return 0;
}