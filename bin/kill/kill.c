#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#define usage_exit() do{printf("kill: [-signal] pid...\n"); exit(1);}while(0)

int main(int argc, char **argv, char **envp) {
    int err, sig, pidi;

    sig = 9;  /* default to SIGKILL */
    pidi = 1; /* index of pid */
    err = 0;
    if(argc <= 1)
        usage_exit();

    /* Parse -signal arg */
    if(argv[1][0] == '-') {
        if(!isdigit(argv[1][1]))
            usage_exit();
        pidi++;
        sig = atoi(argv[1] + 1);
    }
    /* no pids listed */
    if(pidi == argc)
        usage_exit();

    for( ;pidi < argc; pidi++) {
        pid_t pid;
        if(!isdigit(argv[pidi][0])) {
            printf("kill: skipping invalid pid: %s\n", argv[pidi]);
            continue;
        }
        pid = (pid_t)atoi(argv[pidi]);
        err = kill(pid, sig);
        if(err)
            printf("kill: failed to kill pid %d: %s\n", (int)pid, strerror(errno));
    }
    return err? EXIT_FAILURE : EXIT_SUCCESS;
}
