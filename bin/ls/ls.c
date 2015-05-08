#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/**
* Usage:
* $ ls
* $ ls $HOME
* $ ls /dev
*/
int main(int argc, char *argv[]) {
    int retval = 0;
    int prints = 0;
    struct dirent *dent = NULL;
    void *dir = opendir((argc == 1)?("."):(argv[1]));
    if(!dir) {
        printf("opendir failed: %s\n", strerror(errno));
        return 1;
    }

    for(;;){
        errno = 0;
        dent = readdir(dir);
        if(!dent) {
            if(errno != 0){
                printf("readdir failed: %s\n", strerror(errno));
                retval = 1;
                break;
            }
            break;
        }
        printf("%s\t\t", dent->d_name);
        if(++prints == 4) {
            prints = 4;
            printf("\n");
        }
    }
    printf("\n");

    closedir(dir);
    return retval;
}