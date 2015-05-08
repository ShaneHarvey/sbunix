#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <sys/getprocs.h>

#define usage_exit() do{printf("ps\n"); exit(1);}while(0)

#define handle_error(msg) \
    do { printf(msg ": %s\n", strerror(errno)); \
         exit(EXIT_FAILURE); } while (0)

int main(int argc, char **argv, char **envp) {
    void *procbuf;
    struct proc_struct *procp;
    size_t buflen = 0x8000, loc = 0;
    ssize_t wrote;

    procbuf = malloc(buflen);
    if(!procbuf) {
        printf("malloc: no memory\n");
    }

    wrote = getprocs(procbuf, buflen);
    if(wrote < 0)
        handle_error("getprocs");

    printf("PID\tCMD\n");
    while(loc < wrote) {
        procp = (void*)(loc + (char*)procbuf);
        printf("%d\t%s\n", procp->pid, procp->cmd);
        loc += sizeof(pid_t) + strlen(procp->cmd) + 1;
    }
    return EXIT_SUCCESS;
}
