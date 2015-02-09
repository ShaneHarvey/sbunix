#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char *getenv(const char *name) {
    int i;

    for(i = 0; __environ[i] != NULL; i++) {
        /* search for name */
    }
    return NULL;
}
