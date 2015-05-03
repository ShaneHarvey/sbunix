#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[], char* envp[]) {
    /*printf("Hello World!\n");*/
    errno = 10;
    exit(122);
    return 244;
}
