#include <stdio.h>
#include <stdarg.h>

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    int written;

    va_start(ap, format);
    written = vsnprintf(str, size, format, ap);
    va_end(ap);

    return written;
}
