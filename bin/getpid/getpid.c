#include <stdlib.h>
#include <stdio.h>

int main() {
    printf("My PID:  %d\n", getpid());
    printf("My PPID: %d\n", getppid());
    return 0;
}