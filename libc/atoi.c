#include <stdlib.h>
#include <ctype.h>

int atoi(const char *nptr) {
    int rv = 0, negate = 0;
    if(!nptr) {
        return 0;
    }
    if(*nptr == '-') {
        negate = 1;
        nptr++;
    }
    while(isdigit(*nptr)) {
        rv = 10 * rv + *nptr - '0';
        nptr++;
    }
    return (negate)?(-1 * rv):(rv);
}