#ifndef _SBUNIX_ASM_H
#define _SBUNIX_ASM_H

/* Header for common assembly routines. */

#define halt_loop(fmt, ...) \
            do { \
                printf(fmt, ##__VA_ARGS__); \
                while(1) { \
                    __asm__ __volatile__ ("hlt;"); \
                } \
            } while(0)

#endif
