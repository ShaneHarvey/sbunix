#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

/**
* "%s"
* "%d"
* "%x"
* "%c"
*/
int scanf(const char *format, ...) {
    va_list ap;
    int stored = 0;
    int stored_string = 0;
    char last_char = 0;
    ssize_t rv = 0;

    va_start(ap, format);

    if(format == NULL) {
        errno = EINVAL;
        return -1;
    }

    while(*format) {
        if(*format == '%') {
            char *cptr;
 //           int *iptr;
//            unsigned int *uiptr;
            switch(format[1]) {
                case 'c':
                    cptr = va_arg(ap, char*);
                    rv = read(STDIN_FILENO, cptr, 1);
                    if(rv < 0) {
                        return -1;
                    } else if (rv == 0) {
                        return stored;
                    }
                    format += 2;
                    stored++;
                    break;
//                case 'd':
//                    iptr = va_arg(ap, int*);
//
//                    format += 2;
//                    stored++;
//                    break;
//                case 'x':
//                    uiptr = va_arg(ap, unsigned int*);
//
//                    format += 2;
//                    stored++;
//                    break;
                case 's':
                    cptr = va_arg(ap, char *);
//                    if(last_char != 0) {
//                        c = last_char;
//                        last_char = 0;
//                    }
                    stored_string = 0;
                    while(1) {
                        rv = read(STDIN_FILENO, &last_char, 1);
                        if(rv < 0) {
                            return -1;
                        } else if (rv == 0) {
                            if(stored_string) {
                                *cptr = '\0';
                                stored++;
                            }
                            return stored;
                        }
                        if(isspace(last_char)) {
                            if(stored_string) {
                                break;
                            }
                        } else {
                            stored_string = 1;
                            *cptr++ = last_char;
                            last_char = 0;
                        }
                    }
                    *cptr = '\0';
                    stored++;
                    format += 2;
                    break;
                default:
                    break;
            }
        } else {
            char c;
            if(last_char != 0) {
                c = last_char;
                last_char = 0;
            } else {
                rv = read(STDIN_FILENO, &c, 1);
                if(rv < 0) {
                    return -1;
                } else if (rv == 0) {
                    return stored;
                }
            }
            if(c != *format) {
                return stored;
            }
            ++format;
        }
    }
    va_end(ap);
    return stored;
}
