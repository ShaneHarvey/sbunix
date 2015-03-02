#include <sys/sbunix.h>
#include <stdarg.h>
#include <string.h>
#include <sys/writec.h>

void printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    if(fmt == NULL)
        return;

    while(*fmt) {
        if(*fmt == '%') {
            int i;
            unsigned int ui;
            long int li;
            unsigned long int uli;
            uint64_t ui64;
            char arr[129] = {0}, *str, *towrite;
            int len;
            switch(fmt[1]) {
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
                    switch(fmt[2]) {
                        case 'd':
                            li = va_arg(ap, long int);
                            itoa(li, 10, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            fmt++;
                            break;
                        case 'o':
                            uli = va_arg(ap, unsigned long int);
                            uitoa(uli, 8, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            fmt++;
                            break;
                        case 'u':
                            uli = va_arg(ap, unsigned long int);
                            uitoa(uli, 10, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            fmt++;
                            break;
                        case 'x':
                            uli = va_arg(ap, unsigned long int);
                            uitoa(uli, 16, arr, sizeof(arr));
                            len = strlen(arr);
                            towrite = arr;
                            fmt++;
                            break;
                        default:
                            len = 2;
                            towrite = (char*) fmt;
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
                    towrite = (char*) fmt;
                    break;
                default:
                    len = 2;
                    towrite = (char*) fmt;
            }
            fmt += 2;
            writec(towrite, len);
        } else {
            writec(fmt, 1);
            ++fmt;
        }
    }
    va_end(ap);
}
