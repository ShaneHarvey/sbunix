#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * Can print at max 4096
 */
int printf(const char *format, ...) {
    va_list ap;
    char printstr[4096];
    int written;

    if(format == NULL || *format == '\0')
        return 0;

    va_start(ap, format);
    written = vsnprintf(printstr, 4096, format, ap);
    va_end(ap);

    return (int)write(STDOUT_FILENO, printstr, written);
}


int vprintf(const char *format, va_list ap) {
    char printstr[4096];
    int written;

    if(format == NULL || *format == '\0')
        return 0;

    written = vsnprintf(printstr, 4096, format, ap);

    return (int)write(STDOUT_FILENO, printstr, written);
}
