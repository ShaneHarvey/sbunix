#ifndef _STDIO_H
#define _STDIO_H

#include <sys/types.h>

int printf(const char *fmt, ...) __attribute__((format(printf,1,2)));
int snprintf(char *str, size_t size, const char *fmt, ...) __attribute__((format(printf,3,4)));
int snprintf(char *str, size_t size, const char *fmt, ...) __attribute__((format(printf,3,4)));

int scanf(const char *format, ...) __attribute__((format(scanf,1,2)));

#endif
