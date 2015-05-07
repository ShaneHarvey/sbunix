#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv, char **envp) {
    pid_t p;

    p = fork();
    if(p < 0) {
        printf("fork failed: %s\n", strerror(errno));
    } else if(p == 0) {
        /* In child */
        printf("In child!\n");
    } else {
        /* In parent */
        printf("In parent, child pid=%d!\n", (int)p);
    }
    printf("In child or parent\n");
    return 0;
}
