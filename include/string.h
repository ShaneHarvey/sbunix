#ifndef _STRING_H
#define _STRING_H

#include <sys/defs.h>
#include <sys/types.h>

#define NULL ((void*)0)

int      memcmp(const void *, const void *, size_t);
void    *memcpy(void *restrict, const void *restrict, size_t);
int      memcmp(const void *, const void *, size_t);
void    *memcpy(void *restrict, const void *restrict, size_t);
void    *memset(void *s, int c, size_t count);
char    *strcat(char *restrict, const char *restrict);
char    *strchr(const char *, int);
int      strcmp(const char *, const char *);
int      strcoll(const char *, const char *);
char    *strcpy(char *restrict, const char *restrict);
size_t   strcspn(const char *, const char *);

char    *strdup(const char *);

char    *strerror(int);

int     *strerror_r(int, char *, size_t);

size_t   strlen(const char *);
char    *strncat(char *restrict, const char *restrict, size_t);
int      strncmp(const char *, const char *, size_t);
char    *strncpy(char *restrict, const char *restrict, size_t);
char    *strpbrk(const char *, const char *);
char    *strrchr(const char *, int);
size_t   strspn(const char *, const char *);
char    *strstr(const char *, const char *);
char    *strtok(char *restrict, const char *restrict);

#endif /* _STRING_H */