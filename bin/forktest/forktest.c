#include <stdlib.h>
#include <stdio.h>

int main() {
    pid_t p;

    p = fork();
    if(p == 0) {
        /* In child */
//        printf("In child!\n");
        write(STDOUT_FILENO, "In child\n", 9);
        sleep(30);
    } else {
        /* In parent */
//        printf("In parent, child pid=%d!\n", (int)p);
        sleep(30);
    }
    return 0;
}
