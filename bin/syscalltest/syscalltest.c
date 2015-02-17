#include <stdlib.h>
#include <stdio.h>

/* syscalls should all come from the original stdlib.h we were given */


/**
* Test some syscalls that weren't tested elsewhere.
*/
int main(int argc, char *argv[], char *envp[]) {
    uint pid, ppid;

    pid = getpid();
    printf("pid: %u\n", pid);

    ppid = getppid();
    printf("ppid: %u\n", ppid);

    printf("Sleeping for 5 seconds...\n");
    /*printf("Alarm in 1 second...\n");
    alarm(1);*/
    printf("left to sleep: %u\n", sleep(5));

    exit(0);
}