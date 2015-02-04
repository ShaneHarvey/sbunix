#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE 1024

int main(int argc, char **argv, char **envp) {
    int finished = 0;
    ssize_t rv;
    char *prompt = "sbush$ ";
    char cmd[MAX_LINE];

    while(!finished) {
        char *cursor;
        char last;

        rv = write(1, prompt, strlen(prompt));
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
        rv = write(1, cmd, strlen(cmd));
        if(rv < 0) {
            break;
        }
    }


    return 0;
}

