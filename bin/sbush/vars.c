#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
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
int _change_val(var_t *var, char *newval) {
    if(newval == NULL) {
        if(var->val != NULL) {
            free(var->val);
        }
        var->val = NULL;
    } else {
        var->val = realloc(var->val, strlen(newval) + 1);
        if(var->val == NULL) {
            return -1;
        }
        strcpy(var->val, newval);
    }
    return 0;
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
* Return 0 on success, -1 on error with errno set to ENOMEM or EINVAL.
*/
int save_var(char *key, char *val) {
    var_t *var;

    if(key == NULL) {
        errno = EINVAL;
        return -1;
    }
    var = _load_var(key);
    if(var != NULL) {
        return _change_val(var, val);
    }
    var = malloc(sizeof(var_t));
    if(var == NULL) {
        return -1;
    }
    var->val = NULL;
    var->prev = NULL;
    var->next = NULL;
    var->key = malloc(strlen(key) + 1);
    if(var->key == NULL) {
        free(var);
        return -1;
    }
    strcpy(var->key, key);

    if(_change_val(var, val) == -1) {
        free(var->key);
        free(var);
        return -1;
    }
    _insert_var(var);
    return 0;
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