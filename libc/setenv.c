#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int do_setup = 1;

/**
* Change environ on the stack to to being stored entirely on the heap
*/
static int setup_environ(void) {
    char **new_environ, *new_str;
    size_t len = 0, size = 0, i;

    while(__environ[size] != NULL) {
        size++;
    }

    new_environ = malloc((size + 1) * sizeof(char *));
    if(new_environ == NULL) {
        return -1;
    }
    for(i = 0; i < size; i++) {
        len = strlen(__environ[i]);
        new_str = malloc(len + 1);
        if(new_str == NULL) {
            break;
        }
        strcpy(new_str, __environ[i]);
        new_environ[i] = new_str;
    }
    if(i < size) {
        /* Malloc failed so free all mem */
        while(i >= 0) {
            free(new_environ[i--]);
        }
        free(new_environ);
        return -1;
    }
    new_environ[i] = NULL;

    __environ = new_environ;
    do_setup = 0;
    return 0;
}

int setenv(const char *envname, const char *envval, int overwrite) {
    char *new_str;
    size_t namelen, vallen;
    char **new_environ;
    int i;

    /* Name cannot be NULL, empty, or contain '=' */
    if(envname == NULL || *envname == '\0' || strchr(envname, '=')) {
        errno = EINVAL;
        return -1;
    }

    /* Return if we cannot overwrite the existing value */
    if(!overwrite && getenv(envname)) {
        return 0;
    }
    if(do_setup) {
        if(setup_environ() < 0) {
            return -1;
        }
    }
    namelen = strlen(envname);
    vallen = envval? strlen(envval) : 0;

    /* create the combine string "envname=envval" */
    new_str = malloc(namelen + vallen + 2);
    if(new_str == NULL) {
        return -1;
    }
    strcpy(new_str, envname);
    new_str[namelen] = '=';
    if(envval == NULL) {
        new_str[namelen + 1] = '\0';
    } else {
        strcpy(new_str + namelen + 1, envval);
    }

    for(i = 0; __environ[i] != NULL; i++) {
        if((strncmp(__environ[i], envname, namelen) == 0) &&
                __environ[i][namelen] == '=') {
            break;
        }
    }
    if(__environ[i] != NULL) {
        /* Replace old value */
        free(__environ[i]);
        __environ[i] = new_str;
    } else {
        /* Need to realloc __environ */
        size_t size = 0;

        while(__environ[size] != NULL) {
            size++;
        }
        new_environ = realloc(__environ, (size + 2) * sizeof(char *));
        if(new_environ == NULL) {
            free(new_str);
            return -1;
        }
        new_environ[size] = new_str;
        new_environ[size + 1] = NULL;
        __environ = new_environ;
    }

    return 0;
}