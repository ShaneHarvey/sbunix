#ifndef _SBUNIX_STRING_H
#define _SBUNIX_STRING_H

#include <sys/types.h>

#define NULL ((void*)0)

int      memcmp(const void *, const void *, size_t);
void    *memcpy(void *restrict dest, const void *restrict src, size_t n);
void    *memmove(void *dest, const void *src, size_t count);
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

size_t   strlen(const char *);
char    *strncat(char *restrict, const char *restrict, size_t);
int      strncmp(const char *, const char *, size_t);
char    *strncpy(char *restrict, const char *restrict, size_t);
char    *strpbrk(const char *, const char *);
char    *strrchr(const char *, int);
size_t   strspn(const char *, const char *);
char    *strstr(const char *, const char *);
char    *strtok(char *restrict, const char *restrict);
char    *strrev(char *str);
char    *itoa(long long val, int base, char *str, size_t len);
char    *uitoa(unsigned long long val, int base, char *str, size_t len);

#endif /* _SBUNIX_STRING_H */