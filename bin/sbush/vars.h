#ifndef _VARS_H
#define _VARS_H

/* TODO non-hardcoded PAGE_SIZE */
#define PAGE_SIZE 4096

void save_var(char *key, char *val);

char *load_var(char *key);

void cleanup_vars(void);

void print_all(void);

void setup_vars(char **argv, char **envp);

char *swap_vars(char *line, size_t size);

char *dynamic_strcat(char *dest, size_t *dest_size, size_t *index, const char *src);

#endif
