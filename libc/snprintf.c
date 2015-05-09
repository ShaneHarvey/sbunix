#include "_snprintf.h"

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    int written;

    va_start(ap, format);
    written = _snprintf(str, size, format, ap);
    va_end(ap);

    return written;
}
