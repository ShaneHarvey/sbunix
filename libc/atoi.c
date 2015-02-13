#include <stdlib.h>
#include <ctype.h>

int atoi(const char *nptr) {
    int rv = 0;
    while(isdigit(*nptr)) {
        rv = 10 * rv + *nptr - '0';
        nptr++;
    }
    return rv;
}