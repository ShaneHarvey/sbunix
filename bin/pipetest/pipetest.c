#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int pipefd[2], err, i;
    char buffer[4096];

    err = pipe(pipefd);
    if(err) {
        printf("pipe failed: %s\n", strerror(errno));
        return 1;
    }
    for(i = 0; i < 20; i++)
        write(pipefd[1], "hello pipe ", 11);
    close(pipefd[1]);
    for(i = 0; i < 100; i++) {
        /* Continue reading from the pipe until it is empty */
        err = read(pipefd[0], buffer, 1024);
        if(err < 0) {
            printf("read error 1: %s\n", strerror(errno));
            break;
        } if(err == 0) {
            printf("Read EOF\n");
            break;
        } else {
            printf("Read %d bytes\n", err);
        }
    }
    return 0;
}
