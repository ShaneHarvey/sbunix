#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>       /* write */
#include <sys/types.h>    /* waitpid */
#include <sys/wait.h>     /* waitpid */
#include <sys/resource.h>     /* waitpid */
#include <limits.h>       /* PATH_MAX */
#include <fcntl.h>        /* open */
#include <errno.h>        /* errno */

char *const sbush_argv[] = {"/bin/sbush", NULL};
char *const sbush_envp[] = {"PATH=/bin:", "HOME=/root", "USER=root", NULL};
volatile int forever = 1;

int main(int argc, char **argv, char **envp) {
    pid_t sbush_pid;
    int reboots = 0;
reboot:
    if(reboots++ >= 500)
        exit(1);
    sbush_pid = fork();
    if(sbush_pid < 0) {
        printf("/bin/init: fork failed: %s\n", strerror(errno));
        exit(1);
    } else if(sbush_pid == 0) {
        /* change to root's home dir and exec shell */
        chdir("/root");
        execve(sbush_argv[0], sbush_argv, sbush_envp);
        printf("/bin/init: execve '%s': %s\n", sbush_argv[0], strerror(errno));
        exit(1);
    }
//    printf("/bin/init: sbush pid=%d!\n", (int)sbush_pid);

    /* Call wait to reap any zombie processes */
    while(forever) {
        int status;
        pid_t wpid;
        wpid = waitpid((pid_t)-1, &status, 0);
        if (wpid == (pid_t)-1) {
            goto reboot;
        } else if(WIFEXITED(status)) {
            status = WEXITSTATUS(status);
            printf("/bin/init: reaped pid %d, exit status: %d\n", (int)wpid, status);
        } else if(WIFSIGNALED(status)) {
            status = WTERMSIG(status);
            printf("/bin/init: reaped pid %d, killed by %s\n", (int)wpid, strsignal(status));
        } else {
            printf("/bin/init: reaped pid %d, unknown exit status: %d\n", (int)wpid, status);
        }
    }
    return EXIT_FAILURE;
}
