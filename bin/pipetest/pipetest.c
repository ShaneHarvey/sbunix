#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int pipefd[2], err, i;
    char buffer[4096];

    err = pipe(pipefd);
    if(err < 0) {
        printf("pipe failed: %s\n", strerror(errno));
        return 1;
    }
    err = dup2(STDOUT_FILENO, pipefd[1]);
    if(err < 0) {
        printf("dup2 failed: %s\n", strerror(errno));
        return 1;
    }

    for(i = 0; i < 20; i++)
        write(pipefd[1], "hello pipe", 10);
    for(i = 0; i < 100; i++) {
        /* Continue reading from the pipe until it is empty */
        err = read(pipefd[0], buffer, 1024);
        if(err < 0) {
            printf("read error: %s\n", strerror(errno));
            break;
        } if(err == 0) {
            printf("Read EOF\n");
            break;
        } else {
            buffer[err] = '\0';
            printf("Read %d bytes: '%s'\n", err, buffer);
        }
    }
    close(pipefd[1]);
    close(pipefd[0]);
    return 0;
}
