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

int main(int argc, char **argv, char **envp) {
    pid_t sbush_pid;

    sbush_pid = fork();
    if(sbush_pid < 0) {
        printf("\n/bin/init: fork failed: %s\n", strerror(errno));
        exit(1);
    } else if(sbush_pid == 0) {
        /* change to root's home dir and exec shell */
        chdir("/root");
        execve(sbush_argv[0], sbush_argv, sbush_envp);
        printf("\n/bin/init: execve '%s': %s\n", sbush_argv[0], strerror(errno));
        exit(1);
    }
//    printf("/bin/init: sbush pid=%d!\n", (int)sbush_pid);

    /* Call wait to reap any zombie processes */
    while(1) {
        int status;
        pid_t wpid;
        wpid = waitpid((pid_t)-1, &status, 0);
        if (wpid == (pid_t)-1) {
            printf("\n/bin/init: waitpid failed: %s\n", strerror(errno));
            break;
        } else if(WIFEXITED(status)) {
            status = WEXITSTATUS(status);
        } else if(WIFSIGNALED(status)) {
            status = WTERMSIG(status);
            printf("\n/bin/init: reaped pid %d, killed by signal %d\n", (int)wpid, status);
        } else {
            status = 1;  /* ? Didn't exit or get killed by signal? */
        }
        printf("\n/bin/init: reaped pid %d, exit status: %d\n", (int)wpid, status);
    }
    printf("\n/bin/init: no more child processes: please reboot SBUnix\n");
    return EXIT_FAILURE;
}
