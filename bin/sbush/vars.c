#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <debug.h>
#include "vars.h"

typedef struct var {
    char *key;
    char *val;
    struct var *prev;
    struct var *next;
} var_t;

static var_t *_head = NULL;

/*
 *
 */
void _insert_var(var_t *newvar) {
    var_t *cur, *last;
    if(newvar == NULL) {
        return;
    }
    /* Insert as head */
    if(_head == NULL) {
        newvar->next = NULL;
        newvar->prev = NULL;
        _head = newvar;
        return;
    }
    /* Insert in lexicographically increasing order */
    for(cur = _head; cur != NULL; cur = cur->next) {
        int rv = strcmp(newvar->key, cur->key);
        if(rv == 0) {
            /* already exists! */
            error("var %s already exists in the list!\n", newvar->key);
        } else if (rv < 0) {
            /* Insert before this  */
            newvar->next = cur;
            newvar->prev = cur->prev;
            if(cur == _head) {
                _head->prev = newvar;
                _head = newvar;
            } else {
                cur->prev->next = newvar;
                cur->prev= newvar;
            }
            return;
        }
        last = cur;
    }
    /* Insert at the end */
    last->next = newvar;
    newvar->next = NULL;
    newvar->prev = last;
}

/*
 * Change the value of var to newval
 */
void _change_val(var_t *var, char *newval) {
    if(newval == NULL) {
        if(var->val != NULL) {
            free(var->val);
        }
        var->val = NULL;
    } else {
        var->val = realloc(var->val, strlen(newval) + 1);
        if(var->val == NULL) {
            printf("malloc failed: %s\n", strerror(errno));
            exit(1);
        }
        strcpy(var->val, newval);
    }
}

var_t *_load_var(char *key) {
    var_t *var = _head;
    if(key == NULL) {
        return NULL;
    }

    for(var = _head; var != NULL && var->key != NULL; var = var->next) {
        int rv = strcmp(key, var->key);
        if(rv == 0) {
            /* found it! */
            return var;
        } else if (rv < 0) {
            /* lexicographically too small to be in the list be found */
            return NULL;
        }
    }
    return NULL;
}

/**
* Save key to value
*/
void save_var(char *key, char *val) {
    var_t *var;

    if(key == NULL) {
        return;
    }
    var = _load_var(key);
    if(var != NULL) {
        _change_val(var, val);
        return;
    }
    var = malloc(sizeof(var_t));
    if(var == NULL)  {
        printf("malloc failed: %s\n", strerror(errno));
        exit(1);
    }
    var->val = NULL;
    var->prev = NULL;
    var->next = NULL;
    var->key = malloc(strlen(key) + 1);
    if(var->key == NULL) {
        printf("malloc failed: %s\n", strerror(errno));
        exit(1);
    }
    strcpy(var->key, key);

    _change_val(var, val);
    _insert_var(var);
}

/**
* returns the value associated with the key
*/
char *load_var(char *key) {
    var_t *var = _load_var(key);
    if(var == NULL) {
        return NULL;
    }
    return var->val;
}


/**
* Free all the memory held by _head
*/
void cleanup_vars(void) {
    var_t *tmp;
    while(_head != NULL) {
        tmp = _head->next;
        if(_head->key != NULL)
            free(_head->key);
        if(_head->val != NULL)
            free(_head->val);
        free(_head);
        _head = tmp;
    }
}

void print_all(void) {
    var_t *cur;
    for(cur = _head; cur != NULL; cur = cur->next) {
        printf("%s=%s\n", cur->key, cur->val);
    }
}


/**
* Adds all the environment variables to the variable cache
*/
void setup_vars(char **argv, char **envp) {
    char *equals;
    int i = 0;
    while(*envp != NULL) {
        equals = strchr(*envp, '=');
        *equals = '\0';
        save_var(*envp, equals + 1);
        *equals = '=';
        envp++;
    }
    while(*argv != NULL) {
        char var_name[2] = {0};
        var_name[0] = '0' + i;
        save_var(var_name, *argv);
        argv++;
        i++;
    }
    /* Initialize $? variable */
    save_var("?", "0");
}

/**
* Swap out variables of the form "$varname" with their value
*/
char *swap_vars(char *line, size_t size) {
    char temp, *dest;
    size_t readi = 0, writei = 0, dest_size = size;

    /* New line with expanded variables */
    dest = malloc(dest_size);
    if(dest == NULL) {
        printf("malloc failed: %s\n", strerror(errno));
        exit(1);
    }
    memset(dest, 0, dest_size);
    while(readi < size && line[readi] != '\0') {
        if(line[readi] == '$') {
            char c = line[readi + 1];
            char *var_name = &line[readi + 1];
            if(isalnum(c) || c == '?' || c == '$' || c == '_') {
                readi += 2;
                if(isalpha(c) || c == '_') {
                    while (isalnum(line[readi]) || line[readi] == '_') {
                        readi++;
                    }
                }
                /* Null terminate var_name */
                temp = line[readi];
                line[readi] = '\0';
                /* Lookup var_name */
                dest = dynamic_strcat(dest, &dest_size, &writei, load_var(var_name));
                /*restore previous character at current readi */
                line[readi] = temp;
                continue;
            }
            /* Just drop to below write the '$' */
        }
        dest[writei++] = line[readi++];
        if(writei + 1 >= dest_size) {
            /* Realloc more space */
            dest_size += PAGE_SIZE;
            dest = realloc(dest, dest_size);
            if(dest == NULL) {
                printf("realloc failed: %s\n", strerror(errno));
                exit(1);
            }
        }
    }
    free(line);
    return dest;
}

/**
* Copy string src to string *dest starting at *index. The destination string
* is realloc'd if index + strlen(src) is larger than dest_size
*/
char *dynamic_strcat(char *dest, size_t *dest_size, size_t *index, const char *src) {
    if(src != NULL) {
        size_t len = strlen(src);
        if(len + *index + 1 >= *dest_size) {
            /* Realloc more space */
            while(len + *index + 1 >= *dest_size) {
                *dest_size += PAGE_SIZE;
            }
            dest = realloc(dest, *dest_size);
            if(dest == NULL) {
                printf("realloc failed: %s\n", strerror(errno));
                exit(1);
            }
        }
        /* copy value to dest */
        strcpy(dest + *index, src);
        *index += len;
    }
    return dest;
}
