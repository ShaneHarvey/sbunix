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

const char *sbush_argv[] = {NULL};
const char *sbush_envp[] = {"PATH=/bin/:", "HOME=/root/", "USER=root", NULL};
char *sbush_path = "/bin/sbush";

int main(int argc, char **argv, char **envp) {
    pid_t sbush_pid;

    sbush_pid = fork();
    if(sbush_pid < 0) {
        printf("/bin/init: fork failed: %s\n", strerror(errno));
        exit(1);
    } else if(sbush_pid == 0) {
        /* change to root's home dir and exec shell */
        chdir("/root");
        execve(sbush_path, argv, envp);
        printf("/bin/init: execve '%s': %s\n", sbush_path, strerror(errno));
        exit(1);
    }
    /* In parent */
    printf("/bin/init: sbush pid=%d!\n", (int)sbush_pid);

    /* Call wait to reap any zombie processes */
    while(1) {
        int status;
        pid_t wpid;
        wpid = waitpid((pid_t)-1, &status, 0);
        if (wpid == (pid_t)-1) {
            printf("/bin/init: waitpid failed: %s\n", strerror(errno));
            break;
        } else {
            if(WIFEXITED(status))
                status = WEXITSTATUS(status);
            else
                status = 1;
            printf("/bin/init: reaped pid %d, exit status: %d\n", (int)wpid, status);
        }
    }
    printf("/bin/init: no more child processes: please reboot SBUnix\n");
    return EXIT_FAILURE;
}
