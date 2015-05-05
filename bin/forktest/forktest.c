#include <stdlib.h>
#include <stdio.h>

int main() {
    pid_t p;

    p = fork();
    if(p == 0) {
        /* In child */
        printf("In child!\n");
    } else {
        /* In parent */
        sleep(1);
//        printf("In parent, child pid=%d!\n", (int)p);
    }
    return 0;
}