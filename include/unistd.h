#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>

extern char **__environ;

/* standard file nums */
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

unsigned int sleep(unsigned int seconds);

typedef long intptr_t;
void *sbrk(intptr_t increment);


int gethostname(char *name, size_t len);

#endif