#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <debug.h>
#include "../../include/dirent.h"

/**
* Usage:
* $ ls
* $ ls $HOME
* $ ls /dev
*/
int main(int argc, char *argv[]) {
    int retval = 0;
    struct dirent *dent = NULL;
    DIR *dir = opendir((argc == 1)?("."):(argv[1]));
    if(!dir) {
        error("opendir failed: %s\n", strerror(errno));
        return 1;
    }

    for(;;){
        errno = 0;
        dent = readdir(dir);
        if(!dent) {
            if(errno != 0){
                error("readdir failed: %s\n", strerror(errno));
                retval = 1;
                break;
            }
            break;
        }
        printf("%s\t", dent->d_name);
    }
    printf("\n");

    closedir(dir);
    return retval;
}