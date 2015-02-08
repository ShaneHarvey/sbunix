#ifndef _UNISTD_H
#define _UNISTD_H

/* standard file nums */
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2


typedef long intptr_t;
void *sbrk(intptr_t increment);


int gethostname(char *name, size_t len);

#endif