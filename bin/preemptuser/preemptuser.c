#include <stdlib.h>
#include <stdio.h>

int main() {
    uint64_t x = 1;
    while(x) {
        if(++x == 0x10000000) {
            printf("User did 0x%lx additions!\n", x);
            x = 1;
        }
    }
    return 0;
}