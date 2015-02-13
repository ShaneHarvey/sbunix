#include <stdlib.h>
#include <ctype.h>

int atoi(const char *nptr) {
    int rv = 0;
    char c;
    while(isdigit((c = *nptr++))) {
        rv = 10 * rv + c - '0';
    }
    return rv;
}