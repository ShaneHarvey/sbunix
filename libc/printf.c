#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

char *strrev(char *str) {
    char *start = str, *end = str, temp;

    if(str == NULL)
        return str;
    while(*end)
        end++;
    end--;
    while(start < end) {
        temp = *end;
        *end-- = *start;
        *start++ = temp;
    }
    return str;
}

int convert(int i, int base, char *arr) {
    int count = 0;
    int neg = 0;

    if(i < 0 && base == 10) {
        neg = 1;
        i *= -1;
    }

    if(i == 0) {
        arr[0] = '0';
        arr[1] = '\0';
        return 1;
    }

    while(i) {
        arr[count++] = (i % base) + '0';
        i /= base;
    }
    /* add negative sign if base-10 */
    if(neg && base == 10) {
        arr[count++] = '-';
    }
    /* null terminate */
    arr[count] = '\0';
    /* reverse string */
    strrev(arr);

    return count;
}

int parseint(char *str, int base) {
    int i = 0;

    if(str == NULL)
        return 0;
    while(*str) {
        i *= base;
        i += *str++ - '0';
    }
    return i;
}

int printf(const char *format, ...) {
	va_list ap;
	int printed = 0;

	va_start(ap, format);

    if(format == NULL)
        return 0;

	while(*format) {
        if(*format == '%') {
            int i;
            char arr[21] = {0}, *str;
            int len;
            switch(format[1]) {
                case 'd':
                    i = va_arg(ap, int);
                    len = convert(i, 10, arr);
                    write(STDOUT_FILENO, arr, len);
                    printed += len;
                    format += 2;
                    break;
                case 'p':
                    /* todo no worky fix convert */
                    i = va_arg(ap, uint64_t);
                    len = convert(i, 16, arr);
                    write(1, arr, len);
                    printed += len;
                    format += 2;
                    break;
                case 's':
                    str = va_arg(ap, char *);
                    len = strlen(str);
                    write(STDOUT_FILENO, str, len);
                    printed += len;
                    format += 2;
                    break;
                case '%':
                    write(STDOUT_FILENO, format, 1);
                    ++printed;
                    format += 2;
                    break;
                default:
                    write(STDOUT_FILENO, format, 1);
                    ++printed;
                    ++format;
            }
        } else {
            write(STDOUT_FILENO, format, 1);
            ++printed;
            ++format;
        }
	}
    va_end(ap);
	return printed;
}
