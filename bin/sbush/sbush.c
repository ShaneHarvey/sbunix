#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_LINE 1024

typedef struct cmd {
    char **argv;
    struct cmd *next;
}cmd_t;

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
* Returns the number of arguments in a line of input. Ends on '\0' or '|'
*/
int argcount(char *line) {
    int argc = 0;
    char *cur, lastchar;

    if(line == NULL) {
        return 0;
    }
    for(cur = line, lastchar = '\0'; *cur != '\0'; lastchar = *cur, ++cur) {
        if(isspace(*cur)) {
            ++argc;
        }
    }
    if(*cur == '\0' && lastchar != '\0') {
        ++argc;
    }

    return argc;
}

/**
* Return an argv array for the input line
*/
char **mkargv(char *line) {
    int argc, i;
    char **argv, *cur, *argstart;

    if(line == NULL) {
        return NULL;
    }
    /* ignore leading whitespace */
    while(isspace(*line)) {
        line++;
    }
    argc = argcount(line);
    argv = malloc((argc + 1) * sizeof(char *));
    if(argv == NULL) {
        printf("malloc failed!\n");
        exit(1);
    }

    i = 0;
    for(cur = argstart = line; *cur != '\0'; ++cur) {
        if(isspace(*cur)) {
            *cur = '\0';
            argv[i++] = argstart;
            argstart = cur + 1;
        }
    }
    if(*argstart != '\0')
        argv[i++] = argstart;
    argv[i] = NULL;
    return argv;
}
/**
* Parse a line of input and create a list of commands to execute
*/
cmd_t *parse_line(char *line) {
    cmd_t *cmd = NULL;
    char *nextstart, *cur;
    char lastchar;

    if(line == NULL) {
        return NULL;
    }
    strip(line);
    if(*line == '\0') {
        return NULL;
    }
    nextstart = NULL;
    /* find the start of the next command in pipeline */
    for(cur = line, lastchar = '\0'; *cur != '\0'; lastchar = *cur, ++cur) {
        if((isspace(lastchar) && *cur == '|')) {
            /* null terminate the line */
            *cur = '\0';
            nextstart = cur + 1;
            break;
        }
    }
    cmd = malloc(sizeof(cmd_t));
    if(cmd == NULL) {
        printf("malloc failed!\n");
        exit(1);
    }
    cmd->argv = mkargv(line);
    cmd->next = parse_line(nextstart);
    return cmd;
}

/**
* Free all the memory in a command list
*/
void freecmd(cmd_t *cmd) {
    cmd_t *tmp;
    while(cmd != NULL) {
        tmp = cmd->next;
        /* free data */
        if(cmd->argv != NULL) {
            free(cmd->argv);
        }
        /* free cmd_t struct */
        free(cmd);
        cmd = tmp;
    }
}

/**
 * Process the cmd inputted by the user
 */
int proccescmd(char *line, char **envp) {
    int rv, infile, outfile, pfd[2];
    cmd_t *cmd, *curcmd;
    cmd = parse_line(line);
    if(cmd == NULL) {
        return 1;
    }
    infile = STDIN_FILENO;
    outfile = STDOUT_FILENO;
    for(curcmd = cmd; curcmd != NULL; curcmd = curcmd->next) {
        if(curcmd->next != NULL) {
            rv = pipe(pfd);
            if(rv < 0) {
                printf("pipe failed!\n");
                exit(1);
            }
            outfile = pfd[1];
        } else {
            outfile = STDOUT_FILENO;
        }
        /**
        * This if/else should be another function
        */
        if(strcmp(curcmd->argv[0], "cd") == 0) {
            /* TODO: what if argv[1] is null */
            rv = chdir(curcmd->argv[1]);
            if(rv) {
                printf("cd: %s: %s\n", curcmd->argv[1], "No such file or directory");
                return 1;
            }
        } else {
            pid_t pid;

            pid = fork();
            if(pid == 0) {
                if(infile != STDIN_FILENO) {
                    dup2(infile, STDIN_FILENO);
                    close(infile);
                }
                if(outfile != STDOUT_FILENO) {
                    dup2(outfile, STDOUT_FILENO);
                    close(outfile);
                }
                /* Setup command and argv */
                rv = execve(curcmd->argv[0], curcmd->argv, envp);
                /* execve failed */
                printf("execve failed!\n");
                exit(rv);
            } else if(pid > 0) {
                int status;
                pid_t wpid;
                /* wait for pid */
                wpid = waitpid(pid, &status, 0);
                if(wpid < 0) {
                    printf("waitpid failed!\n");
                    exit(1);
                }
                printf("sbush: %s finished with status %d\n", curcmd->argv[0], status);
            } else {
                printf("sbush: fork failed!\n");
                return 1;
            }
        }
        /* close used ends of pipes */
        if(infile != STDIN_FILENO) {
            close(infile);
        }
        if(outfile != STDOUT_FILENO) {
            close(outfile);
        }
        infile = pfd[0]; /* next command should read prev output */
    }
    return 1;
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

