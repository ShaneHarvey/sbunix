#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/**
 * Append the dest string with the source string starting at off.
 * If the result will not fit in dest the string is reallocated to
 * accommodate src.
 *
 * @dest: result string, MUST be a pointer returned by malloc
 * @dlen: length of result
 * @src:  string to insert
 * @slen: length of source
 * @off:  index into dest to insert the src string
 */
char *strinsert(char *dest, size_t *dlen, const char *src, size_t slen, size_t *off) {
    size_t newlen;

    if(!dest || !src || !dlen || !off)
        return dest;

    newlen = *off + slen;
    if(newlen > *dlen) {
        /* reallocate dest to twice the required size */
        char *newdest;
        newlen *= 2;
        newdest = realloc(dest, newlen);
        if(!newdest)
            return NULL;
        *dlen = newlen;
        dest = newdest;
    }
    /* safe to copy src into dest */
    strncpy(dest + *off, src, slen+1);
    *off += slen;
    return dest;
}

/**
 * Sane version of sprintf, it allocates a string. User must free it
 */
char *strprintf(const char *format, va_list ap) {
    char *result, *tmpstr;
    size_t newlen, newoff = 0;

    if(format == NULL)
        return NULL;

    newlen = 2*strlen(format); /* initially 2x format string length */
    result = malloc(newlen + 1);
    if(result == NULL)
        return NULL;

    while(*format) {
        char arr[129] = {0};
        const char *towrite;
        size_t len;

        if(*format == '%') {
            int i;
            unsigned int ui;
            long int li;
            unsigned long int uli;
            uint64_t ui64;
            char *str;
            switch(format[1]) {
                case 'c':
                    i = va_arg(ap, int);
                    arr[0] = (char) i;
                    len = 1;
                    towrite = arr;
                    break;
                case 'd':
                    i = va_arg(ap, int);
                    itoa((long long) i, 10, arr, sizeof(arr));
                    len = strlen(arr);
                    towrite = arr;
                    break;
                case 'l':
                    switch(format[2]) {
                        case 'd':
                            li = va_arg(ap, long int);
                            itoa(li, 10, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            format++;
                            break;
                        case 'o':
                            uli = va_arg(ap, unsigned long int);
                            uitoa(uli, 8, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            format++;
                            break;
                        case 'u':
                            uli = va_arg(ap, unsigned long int);
                            uitoa(uli, 10, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            format++;
                            break;
                        case 'x':
                            uli = va_arg(ap, unsigned long int);
                            uitoa(uli, 16, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            format++;
                            break;
                        default:
                            len = 2;
                            towrite = (char*)format;
                    }
                    break;
                case 'p':
                    ui64 = va_arg(ap, uint64_t);
                    if(ui64 == 0) {
                        strcpy(arr, "(nil)");
                    } else {
                        arr[0] = '0';
                        arr[1] = 'x';
                        uitoa(ui64, 16, arr + 2, sizeof(arr));
                    }
                    len = strlen(arr);
                    towrite = arr;
                    break;
                case 'o':
                    ui = va_arg(ap, unsigned int);
                    uitoa((uint64_t)ui, 8, arr, sizeof(arr));
                    len = strlen(arr);
                    towrite = arr;
                    break;
                case 's':
                    str = va_arg(ap, char *);
                    len = strlen(str);
                    towrite = str;
                    break;
                case 'u':
                    ui = va_arg(ap, unsigned int);
                    uitoa((uint64_t) ui, 10, arr, sizeof(arr));
                    len = strlen(arr);
                    towrite = arr;
                    break;
                case 'x':
                    ui = va_arg(ap, unsigned int);
                    uitoa((uint64_t)ui, 16, arr, sizeof(arr));
                    len = strlen(arr);
                    towrite = arr;
                    break;
                case '%':
                    len = 1;
                    towrite = (char*)format;
                    break;
                default:
                    len = 2;
                    towrite = (char*)format;
            }
            format += 2;
        } else {
            towrite = format++;
            len = 1;
        }
        /* Insert the formatted string into the result */
        tmpstr = strinsert(result, &newlen, towrite, len, &newoff);
        if(!tmpstr)
            goto out_newstr;
        result = tmpstr;
    }

    return result;
out_newstr:
    free(result);
    return NULL;
}

int printf(const char *format, ...) {
    va_list ap;
    char *printstr;
    int written;

    va_start(ap, format);

    if(format == NULL || *format == '\0')
        return 0;

    printstr = strprintf(format, ap);
    if(!printstr)
        return -1;

    written = (int)write(STDOUT_FILENO, printstr, strlen(printstr));
    free(printstr); /* free the constructed string */

    va_end(ap);
    return written;
}
