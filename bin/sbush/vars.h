#ifndef _VARS_H
#define _VARS_H

int save_var(char *key, char *val);

char *load_var(char *key);

void cleanup_vars(void);

void print_all(void);

#endif
