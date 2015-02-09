#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char *getenv(const char *name) {
    int i, len;
    if(name == NULL || *name == '\0') {
        return NULL;
    }
    len = strlen(name);
    /* search for name */
    for(i = 0; __environ[i] != NULL; i++) {
        char *eptr = strchr(__environ[i], '=');

        /* length and bytes are equal */
        if(len == eptr - __environ[i] && !strncmp(name, __environ[i], len)) {
            return eptr + 1;
        }
    }
    return NULL;
}
