#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024

/**
* Print the shell prompt from the PS1 string provided
*/
int printp(char *ps1) {
    int rv;

    rv = write(1, ps1, strlen(ps1));
    return rv;
}

/**
* Strips leading and trailing whitespace from a string.
*/
void strip(char *str) {
    char *p, *t;
    for(p = str; isspace(*p); p++);
    for(t = str; *p; t++, p++) {
        *t = *p;
        if(t < p)
            *p = ' ';
    }
    if(p > str) {
        for(; isspace(*(p-1)); p--);
        *p = '\0';
    }
}

/**
* Process the cmd inputted by the user
*/
int proccescmd(char *cmd, char **envp) {
    int rv;
    strip(cmd);
    if(strncmp(cmd, "cd ", 3) == 0) {
        char *path = cmd + 3;
        rv = chdir(path);
        if(rv) {
            printf("cd: %s: %s\n", path, "No such file or directory");
            return 1;
        }
    } else {
        pid_t pid;

        pid = fork();
        if(pid == 0) {
            rv = execve(cmd, NULL, envp);
            /* execve failed */
            exit(rv);
        } else if(pid > 0) {
            int status;
            pid_t wpid;
            wpid = waitpid(pid, &status, 1);
            printf("sbush: cmd finished\n");
        } else {
            printf("sbush: fork failed!\n");
            return 1;
        }
    }
    return rv;
}

int main(int argc, char **argv, char **envp) {
    int finished = 0;
    ssize_t rv;
    char *ps1 = "sbush$ ";
    char cmd[MAX_LINE];

    while(!finished) {
        char *cursor;
        char last;

        rv = printp(ps1);
        if(rv < 0) {
            break;
        }
        for(cursor = cmd, last = '\0'; last != '\n' &&
                cursor < cmd + MAX_LINE - 1 ; ++cursor) {
            rv = read(0, cursor, 1);
            if(rv <= 0) {
                finished = 1;
                break;
            }
            last = *cursor;
            /* Handle special characters eg ctrl, arrow keys */
        }
        *cursor = '\0';

        rv = proccescmd(cmd, envp);
        if(rv < 0) {
            break;
        }
    }
    return 0;
}

