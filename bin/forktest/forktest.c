#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main() {
    pid_t p;

    p = fork();
    if(p < 0) {
        printf("fork failed: %s\n", strerror(errno));
    } else if(p == 0) {
        /* In child */
        printf("In child!\n");
        sleep(2);
    } else {
        /* In parent */
        sleep(3);
        printf("In parent, child pid=%d!\n", (int)p);
    }
    return 0;
}
