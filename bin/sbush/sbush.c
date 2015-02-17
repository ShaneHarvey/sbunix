#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>        /* isspace */
#include <unistd.h>       /* write */
#include <sys/types.h>    /* waitpid */
#include <sys/wait.h>     /* waitpid */
#include <limits.h>       /* PATH_MAX */
#include <fcntl.h>        /* open */
#include <errno.h>        /* errno */
#include "vars.h"         /* save_var/load_var */

#define MAX_LINE 1024

typedef struct cmd {
    int argc;
    char **argv;
    struct cmd *next;
}cmd_t;

int ignore_line(char *line);
void print_prompt(char *ps1);
void strip(char *str);
int arg_count(char *line);
char **build_argv(char *line, int argc);
cmd_t *parse_line(char *line);
void free_cmd(cmd_t *cmd);
int build_path(char *prog, char *fullpath);
void exec_cmd(cmd_t *cmd, int infile, int outfile, char **envp);
int procces_cmd(cmd_t *cmd, char **envp);
int eval_assignment(cmd_t *cmd);
void save_cmd_info(cmd_t *cmd, int exit_status);

int main(int argc, char **argv, char **envp) {
    int finished = 0, interactive = 1;
    ssize_t rv;
    char *ps1;
    char *line;
    size_t line_size = 8;
    int inputfd = STDIN_FILENO;

    if(argc > 1) {
        interactive = 0;
        inputfd = open(argv[1], O_RDONLY);
        if(inputfd < 0) {
            printf("failed to open %s: %s\n", argv[1], strerror(errno));
        }
    }

    line = malloc(line_size);
    if(line == NULL) {
        printf("malloc failed: %s\n", strerror(errno));
        return 1;
    }

    /* Setup vars */
    setup_vars(argv, envp);

    ps1 = load_var("PS1");
    if(ps1 == NULL) {
        /* default prompt */
        ps1 = "[\\u@\\h \\w]$ ";
        save_var("PS1", ps1);
    }

    while(!finished) {
        int i;
        char last_char;
        cmd_t *cmd;

        if(interactive) {
            print_prompt(ps1);
        }
        i = 0;
        last_char = '\0';
        for(i = last_char = 0; last_char != '\n'; last_char = line[i++]) {
            if(i == line_size - 1) {
                line_size *= 2;
                line = realloc(line, line_size);
                if(line == NULL) {
                    printf("malloc failed: %s\n", strerror(errno));
                    exit(1);
                }
            }
            rv = read(inputfd, line + i, 1);
            if(rv <= 0) {
                finished = 1;
                break;
            }
            /* TODO: Handle special characters eg ctrl, arrow keys, tab */
        }
        line[i] = '\0';
        if(ignore_line(line)) {
            continue;
        }
        line = swap_vars(line, line_size);
        cmd = parse_line(line);
        if(cmd == NULL) {
            continue;
        }
        rv = procces_cmd(cmd, envp);
        free_cmd(cmd);
        if(rv < 0) {
            break;
        }
        ps1 = load_var("PS1");
    }
    free(line);
    cleanup_vars();
    if(inputfd != STDIN_FILENO) {
        close(inputfd);
    }
    return 0;
}

/**
* Returns 1 if this line should be ignored
*/
int ignore_line(char *line) {
    if(line == NULL || *line == '\0') {
        return 1;
    }
    while(isspace(*line)) {
        line++;
    }
    if(*line == '#' || *line == '\n' || *line == '\0') {
        return 1;
    }
    return 0;
}

/**
* Process the cmd inputted by the user
* return -1 on user exit or 0
*/
int procces_cmd(cmd_t *cmd, char **envp) {
    int rv, infile, outfile, pfd[2];
    cmd_t *curcmd;

    if(cmd == NULL) {
        return 1;
    }
    infile = STDIN_FILENO;
    outfile = STDOUT_FILENO;
    for(curcmd = cmd; curcmd != NULL; curcmd = curcmd->next) {
        int status = 0;
        if(curcmd->next != NULL) {
            rv = pipe(pfd);
            if(rv < 0) {
                printf("pipe failed: %s\n", strerror(errno));
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
            if(curcmd->argv[1] != NULL) {
                rv = chdir(curcmd->argv[1]);
                if(rv < 0) {
                    printf("cd: %s: %s\n", curcmd->argv[1], strerror(errno));
                    status = 1;
                }
            }
        } else if(strcmp(curcmd->argv[0], "exit") == 0) {
            /* TODO: exit with error code */
            exit(0);
        } else if(strcmp(curcmd->argv[0], "export") == 0) {
            /* Expose the variable to the child */
            char *value = load_var(curcmd->argv[1]);
            if(value != NULL) {
                rv = setenv(curcmd->argv[1], value, 1);
                if (rv < 0) {
                    printf("export %s: %s\n", curcmd->argv[1], strerror(errno));
                    status = 1;
                }
            } else {
                status = 1;
            }
        } else if((rv = eval_assignment(curcmd))) {
            if(rv < 0) {
                printf("assignment failed: %s\n", strerror(errno));
                status = 1;
            }
        } else {
            pid_t pid;

            pid = fork();
            if(pid == 0) {
                /* Does not return */
                exec_cmd(curcmd, infile, outfile, envp);
            } else if(pid > 0) {
                pid_t wpid;
                /* wait for pid */
                wpid = waitpid(pid, &status, 0);
                if(wpid < 0) {
                    printf("waitpid failed: %s\n", strerror(errno));
                    exit(1);
                }
                //printf("sbush: %s finished with status %d\n", curcmd->argv[0], status);
            } else {
                printf("sbush: fork failed: %s\n", strerror(errno));
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

        /* Save info about previous command to the environment */
        save_cmd_info(curcmd, status);
    }
    return 1;
}

/**
* Evaluate the cmd as a "xsa=123" assignment
* returns -1 on error
*          0 if the cmd is not an assignment
*          1 if the assignment was successful
*/

int eval_assignment(cmd_t *cmd) {
    char *equals, *argv0, *tmp;

    argv0 = cmd->argv[0];
    equals = strchr(argv0, '=');
    if(equals == NULL ||                         /* no equals sign */
            equals == argv0 ||                   /* first digit is '=' */
            (!isalpha(*argv0) && *argv0 != '_')) { /* first digit is not [_A-Za-Z] */
        return 0; /* Not an assignment */
    }
    /* Variable name must be [_A-Za-z0-9] */
    for(tmp = argv0 + 1; tmp < equals; tmp++) {
        if((!isalnum(*tmp) && *tmp != '_')) {
            return 0;
        }
    }
    *equals = '\0';

    save_var(argv0, equals + 1);
    return 1;
}

/**
* Executes the non-builtin command cmd.
*/
void exec_cmd(cmd_t *cmd, int infile, int outfile, char **envp) {
    char filename[PATH_MAX]; /* holds full path to program */

    if(infile != STDIN_FILENO) {
        dup2(infile, STDIN_FILENO);
        close(infile);
    }
    if(outfile != STDOUT_FILENO) {
        dup2(outfile, STDOUT_FILENO);
        close(outfile);
    }

    if(build_path(cmd->argv[0], filename) == 1) {
        /* Setup command and argv */
        execve(filename, cmd->argv, __environ);
        /* execve failed */
        printf("execve: %s failed: %s\n", filename, strerror(errno));
        free_cmd(cmd);
        exit(126);
    } else {
        printf("%s: command not found\n", cmd->argv[0]);
        free_cmd(cmd);
        exit(127);
    }

}

/**
* Searches PATH for the program to run and builds the full path for execve
*/
int build_path(char *prog, char *fullpath) {
    char *slash = strchr(prog, '/');

    /* Locate the program to run */
    if(slash == NULL) {
        char *path = load_var("PATH");
        int len = 0;
        while(*path != '\0') {
            int fd;
            len = 0;
            while(path[len] != ':' && path[len] != '\0') {
                len++;
            }
            /* construct full path */
            strncpy(fullpath, path, len);
            fullpath[len] = '/';
            strcpy(fullpath + len + 1, prog);
            fd = open(fullpath, O_RDONLY);
            if(fd >= 0) {
                /* File exists */
                close(fd);
                return 1;
            }
            /* try the next directory in PATH */
            path += len;
            if(path[len] == ':') {
                path++;
            }
        }
    } else if(prog[0] == '/' || (prog[0] == '.' && prog[1] == '/')) {
        strcpy(fullpath, prog);
        return 1;
    } else {
        strcpy(fullpath, "./");
        strcpy(fullpath+2, prog);
        return 1;
    }

    return 0;
}

/**
* Saves info about the previous command.
*/
void save_cmd_info(cmd_t *cmd, int exit_status) {
    char str_status[4] = {0};
    exit_status &= 0xFF;
    if(exit_status == 0) {
        save_var("?", "0");
    } else {
        int i = 0, j = 1;
        while(exit_status != 0) {
            str_status[i++] = '0' + (exit_status % 10);
            exit_status /= 10;
        }
        if(exit_status > 99) {
            j = 2;
        }
        i = str_status[0];
        str_status[0] = str_status[j];
        str_status[j] = (char)i;
        save_var("?", str_status);
    }
    save_var("_", cmd->argv[0]);
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
        printf("malloc failed: %s\n", strerror(errno));
        exit(1);
    }
    cmd->argc = arg_count(line);
    cmd->argv = build_argv(line, cmd->argc);
    cmd->next = parse_line(nextstart);
    return cmd;
}

/**
* Free all the memory in a command list
*/
void free_cmd(cmd_t *cmd) {
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
* Return an argv array for the input line
*/
char **build_argv(char *line, int argc) {
    int i;
    char **argv, *cur, *argstart;

    if(line == NULL) {
        return NULL;
    }
    /* ignore leading whitespace */
    while(isspace(*line)) {
        line++;
    }
    argc = arg_count(line);
    argv = malloc((argc + 1) * sizeof(char *));
    if(argv == NULL) {
        printf("malloc failed: %s\n", strerror(errno));
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
* Returns the number of arguments in a line of input. Ends on '\0' or '|'
*/
int arg_count(char *line) {
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
* Print the shell prompt from the PS1 string provided, special characters include:
* \u – Username
* \h – Hostname
* \w – Full path of the current working directory
*/
void print_prompt(char *ps1) {
    char *tmp = ps1;

    while(*tmp != '\0') {
        if(*tmp == '\\') {
            char buf[PATH_MAX];
            char *user;
            switch (*(tmp + 1)) {
                case 'u':
                    user = load_var("USER");
                    if(user != NULL) {
                        write(STDOUT_FILENO, user, strlen(user));
                    }
                    tmp++;
                    break;
                case 'h':
                    gethostname(buf, PATH_MAX);
                    write(STDOUT_FILENO, buf, strlen(buf));
                    tmp++;
                    break;
                case 'w':
                    getcwd(buf, PATH_MAX);
                    write(STDOUT_FILENO, buf, strlen(buf));
                    tmp++;
                    break;
                default:
                    write(STDOUT_FILENO, "\\", 1);
            }
        } else {
            write(STDOUT_FILENO, tmp, 1);
        }
        tmp++;
    }
}

/**
* Strips leading and trailing whitespace from a string.
*/
void strip(char *str) {
    int start, len;

    if(str == NULL) {
        return;
    }
    len = strlen(str);
    /* Strip trailing whitespace */
    while(len > 0 && isspace(str[len-1])) {
        len--;
    }
    str[len] = '\0';
    /* Strip leading whitespace */
    for(start = 0; start < len && isspace(str[start]); start++);
    if(start != 0) {
        int i = 0;
        while(start < len) {
            str[i++] = str[start++];
        }
        str[i] = '\0';
    }
}
