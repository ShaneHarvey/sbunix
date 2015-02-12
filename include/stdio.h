#ifndef _STDIO_H
#define _STDIO_H

int printf(const char *format, ...) __attribute__((format(printf,1,2)));
int scanf(const char *format, ...) __attribute__((format(scanf,1,2)));

#endif
