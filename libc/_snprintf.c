#include "_snprintf.h"
#include <stdlib.h>
#include <string.h>

/**
 * Snprintf
 *
 * returns number of bytes written
 */
int _snprintf(char *str, size_t size, const char *format, va_list ap) {
    int written = 0;

    if(format == NULL)
        return 0;
    if(!*format) {
        *str = '\0';
        return 1;
    }

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
            char *strarg;
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
                    strarg = va_arg(ap, char *);
                    len = strlen(strarg);
                    towrite = strarg;
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
        /* add the towrite string to the buffer */
        while(written < size && len--) {
            written++;
            *str++ = *towrite++;
        }
        if(written == size)
            break;
    }
    *str = '\0'; /* Always null-terminated */
    return written;
}
