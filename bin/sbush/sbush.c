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
/* TODO non-hardcoded PAGE_SIZE */
#define PAGE_SIZE 4096

typedef struct cmd {
    int argc;
    char **argv;
    struct cmd *next;
}cmd_t;

void setup_vars(char **envp);
char *swap_vars(char *line, size_t size);
int print_prompt(char *ps1);
void strip(char *str);
int arg_count(char *line);
char **build_argv(char *line, int argc);
cmd_t *parse_line(char *line);
void free_cmd(cmd_t *cmd);
int build_path(char *prog, char *fullpath);
void exec_cmd(cmd_t *cmd, int infile, int outfile, char **envp);
int procces_cmd(cmd_t *cmd, char **envp);
int eval_assignment(cmd_t *cmd);

int main(int argc, char **argv, char **envp) {
    int finished = 0;
    ssize_t rv;
    char *ps1;
    char *line;
    int line_size = 8;

    line = malloc(line_size);
    if(line == NULL) {
        printf("malloc failed: %s\n", strerror(errno));
        return 1;
    }

    /* Setup vars */
    setup_vars(envp);

    ps1 = load_var("PS1");
    if(ps1 == NULL) {
        /* default prompt */
        ps1 = "[\\u@\\h \\w]$ ";
    }

    while(!finished) {
        int i;
        char last_char;
        cmd_t *cmd;

        rv = print_prompt(ps1);
        if(rv < 0) {
            break;
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
            rv = read(0, line + i, 1);
            if(rv <= 0) {
                finished = 1;
                break;
            }
            /* TODO: Handle special characters eg ctrl, arrow keys, tab */
        }
        line[i] = '\0';
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
    }
    free(line);
    cleanup_vars();
    return 0;
}

/**
* Adds all the environment variables to the variable cache
*/
void setup_vars(char **envp) {
    char *equals;
    while(*envp != NULL) {
        equals = strchr(*envp, '=');
        *equals = '\0';
        if(save_var(*envp, equals + 1) < 0) {
            printf("malloc failed: %s\n", strerror(errno));
            exit(1);
        }
        *equals = '=';
        envp++;
    }
}

/**
* Swap out variables of the form "$varname" with their value
*
* param
*/
char *swap_vars(char *line, size_t size) {
    char temp, *dest;
    size_t readi = 0, writei = 0, dest_size = size;

    /* New line with expanded variables */
    dest = malloc(dest_size);
    if(dest == NULL) {
        printf("malloc failed: %s\n", strerror(errno));
        exit(1);
    }
    memset(dest, 0, dest_size);
    while(readi < size && line[readi] != '\0') {
        if(line[readi] == '$') {
            char next = line[readi + 1];
            if(isalpha(next) || next == '_') {
                char *var_name = &line[readi + 1];
                char *var_value;
                readi += 2;
                /* Null terminate var_name */
                while(isalnum(line[readi]) || line[readi] == '_') {
                    readi++;
                }
                temp = line[readi];
                line[readi] = '\0';
                /* Lookup var_name */
                var_value = load_var(var_name);
                if(var_value != NULL) {
                    size_t len = strlen(var_value);
                    if(len + writei + 1 >= dest_size) {
                        /* Realloc more space */
                        while(len + writei + 1 >= dest_size) {
                            dest_size += PAGE_SIZE;
                        }
                        dest = realloc(dest, dest_size);
                        if(dest == NULL) {
                            printf("realloc failed: %s\n", strerror(errno));
                            exit(1);
                        }
                    }
                    /* copy value to dest */
                    strcpy(dest + writei, var_value);
                    writei += len;
                }
                /*restore previous character at current readi */
                line[readi] = temp;
                continue;
            } else if (isdigit(next)) {
                /* TODO: add $0,$1,...,$9 for argv[0],argv[1]...,argv[9] */
                continue;
            } else if (next == '?'){
                /* TODO: add last exit status */
                continue;
            } else if (next == '$'){
                /* TODO: add pid */
                continue;
            } else {
                /* Just drop to below write the '$' */

            }
        } else {
            dest[writei++] = line[readi++];
            if(writei + 1 >= dest_size) {
                /* Realloc more space */
                dest_size += PAGE_SIZE;
                dest = realloc(dest, dest_size);
                if(dest == NULL) {
                    printf("realloc failed: %s\n", strerror(errno));
                    exit(1);
                }
            }
        }
    }
    free(line);
    return dest;
}

/**
* Copy string src to string *dest starting at *index. The destination string
* is realloc'd if index + strlen(src) is larger than dest_size
*/
char *dynamic_strcat(char **dest, size_t *dest_size, size_t *index, const char *src) {
    return *dest;
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
                    printf("cd: %s: %s\n", curcmd->argv[1], "No such file or directory");
                }
            }
        } else if(strcmp(curcmd->argv[0], "exit") == 0) {
            return -1;
        } else if((rv = eval_assignment(curcmd))) {
            if(rv < 0) {
                printf("assignment failed: %s\n", strerror(errno));
            }
        } else {
            pid_t pid;

            pid = fork();
            if(pid == 0) {
                /* Does not return */
                exec_cmd(curcmd, infile, outfile, envp);
            } else if(pid > 0) {
                int status;
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

    return save_var(argv0, equals + 1)? -1 : 1;
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
        execve(filename, cmd->argv, envp);
        /* execve failed */
        printf("execve: %s failed: %s\n", filename, strerror(errno));
        free_cmd(cmd);
        exit(-1);
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
    /* Locate the program to run */
    if(prog[0] == '/' || prog[0] == '.' ) {
        strcpy(fullpath, prog);
        return 1;
    } else {
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
    }
    return 0;
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
int print_prompt(char *ps1) {
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
    return 1;
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
        for(; isspace(*(p - 1)); p--);
        *p = '\0';
    }
}
