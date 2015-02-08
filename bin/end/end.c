#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char etext, edata, end; /* The symbols must have some type, or "gcc -Wall" complains */

int main(int argc, char *argv[], char *envp[]) {
    void *sbrkret = sbrk(0);
    int brkret = brk(&end);

    printf("First address past:\n");
    printf("    program text (etext)      %p\n", &etext);
    printf("    initialized data (edata)  %p\n", &edata);
    printf("    uninitialized data (end)  %p\n", &end);

    /* trying to see how/if end and brk's relate */
    printf("    sbrk(0)                   %p\n", sbrkret);
    printf("    brk(&end)                 %d\n", brkret);
    exit(0);
}
