#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
    int pipefd[2], err, i;
    char buffer[4096];

    pipe(pipefd);
//    close(pipefd[0]);
//    for(i = 0; i < 100; i++) {
        /* Continue writing to the pipe until it is full */
        err = write(pipefd[1], buffer, 4096);
        if(err < 0) {
            printf("Write error 1: %s\n", strerror(errno));
//            break;
        }
//    }
    close(pipefd[1]);
    for(i = 0; i < 100; i++) {
        /* Continue reading from the pipe until it is empty */
        err = read(pipefd[0], buffer, 4096);
        if(err < 0) {
            printf("read error 1: %s\n", strerror(errno));
            break;
        } if(err == 0) {
            printf("Read EOF\n");
            break;
        } else {
            printf("Read %d bytes\n", err);
        }
    } err = read(pipefd[0], buffer, 4096);
    if(err < 0) {
        printf("read error 1: %s\n", strerror(errno));
    } if(err == 0) {
        printf("Read EOF\n");
    } else {
        printf("Read %d bytes\n", err);
    }
    /* Write to the read end of a pipe */
    err = write(pipefd[0], "hello", 5);
    if(err < 0) {
        printf("Write error 2: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}
