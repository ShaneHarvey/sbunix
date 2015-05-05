#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int printf(const char *format, ...) {
    va_list ap;
    int printed = 0;

    va_start(ap, format);

    if(format == NULL)
        return 0;

    while(*format) {
        if(*format == '%') {
            int i;
            unsigned int ui;
            long int li;
            unsigned long int uli;
            uint64_t ui64;
            char arr[129] = {0}, *str, *towrite;
            int len;
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
            i = (int)write(STDOUT_FILENO, towrite, len);
            if(i < 0) {
                return -1;
            }
            printed += len;
        } else {
            write(STDOUT_FILENO, format, 1);
            ++printed;
            ++format;
        }
    }
    va_end(ap);
    return printed;
}
