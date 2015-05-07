#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#define usage_exit() do{printf("sleep: NUMBER\n"); exit(1);}while(0)

int main(int argc, char **argv, char **envp) {
    int secs, err;

    if(argc <= 1 || !isdigit(argv[1][0]))
        usage_exit();

    /* Parse NUMBER arg */
    secs = atoi(argv[1]);

    err = sleep(secs);
    if(err)
        printf("sleep: %s\n", strerror(errno));
    return errno;
}
