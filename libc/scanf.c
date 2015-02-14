#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#define xdigitval(c) (isdigit(c)? ((c) - '0') : isupper(c)? ((c) - 'A' + 10) : ((c) - 'a' + 10))
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
    unsigned char last_char = 0;
    ssize_t rv = 0;

    va_start(ap, format);

    if(format == NULL) {
        errno = EINVAL;
        return -1;
    }

    while(*format) {
        if(*format == '%') {
            int neg;
            char *cptr;
            int *iptr;
            unsigned int *uiptr;
            switch(format[1]) {
                case 'c':
                    cptr = va_arg(ap, char*);
                    if(last_char != 0) {
                        *cptr = last_char;
                        last_char = 0;
                    } else {
                        rv = read(STDIN_FILENO, cptr, 1);
                        if(rv < 0) {
                            return -1;
                        } else if (rv == 0) {
                            return stored;
                        }
                    }
                    format += 2;
                    stored++;
                    break;
                case 'd':
                    iptr = va_arg(ap, int*);
                    neg = 0;
                    while(last_char == 0 || isspace(last_char)) {
                        rv = read(STDIN_FILENO, &last_char, 1);
                        if(rv < 0) {
                            return -1;
                        } else if (rv == 0) {
                            return stored;
                        }
                    }
                    /* Test first char for - or digit or something else*/
                    if(last_char == '-') {
                        neg = 1;
                        *iptr = 0;
                    } else if(isdigit(last_char)) {
                        *iptr = last_char - '0';
                    } else {
                        return stored;
                    }
                    do {
                        rv = read(STDIN_FILENO, &last_char, 1);
                        if(rv < 0) {
                            return -1;
                        } else if (rv == 0) {
                            return stored + 1;
                        }
                        if(isdigit(last_char)) {
                            *iptr = *iptr * 10 + last_char - '0';
                        }
                    } while (isdigit(last_char));
                    if(neg) {
                        *iptr *= -1;
                    }
                    format += 2;
                    stored++;
                    break;
                case 'x':
                    uiptr = va_arg(ap, unsigned int*);

                    while(last_char == 0 || isspace(last_char)) {
                        rv = read(STDIN_FILENO, &last_char, 1);
                        if(rv < 0) {
                            return -1;
                        } else if (rv == 0) {
                            return stored;
                        }
                    }
                    /* Test first char for hexdigit or something else*/
                    if(isxdigit(last_char)) {
                        *uiptr = xdigitval(last_char);
                    } else {
                        return stored;
                    }
                    do {
                        rv = read(STDIN_FILENO, &last_char, 1);
                        if(rv < 0) {
                            return -1;
                        } else if (rv == 0) {
                            return stored + 1;
                        }
                        if(isxdigit(last_char)) {
                            *uiptr = *uiptr * 16 + xdigitval(last_char);
                        }
                    } while (isxdigit(last_char));

                    format += 2;
                    stored++;
                    break;
                case 's':
                    cptr = va_arg(ap, char *);
                    if(last_char != 0) {
                        *cptr++ = last_char;
                        last_char = 0;
                    }
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
