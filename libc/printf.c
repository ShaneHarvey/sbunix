#include <stdio.h>
#include <stdlib.h>
#include "_snprintf.h"

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
    written = _snprintf(printstr, 4096, format, ap);
    va_end(ap);

    return (int)write(STDOUT_FILENO, printstr, written);
}
