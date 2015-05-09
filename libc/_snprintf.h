#ifndef _LIBC_SNPRINTF_H
#define _LIBC_SNPRINTF_H

#include <sys/types.h>
#include <stdarg.h>

int _snprintf(char *str, size_t size, const char *format, va_list ap);

#endif //_LIBC_SNPRINTF_H
