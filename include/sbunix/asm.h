#ifndef _SBUNIX_ASM_H
#define _SBUNIX_ASM_H

/* Header for common assembly routines. */

#define halt_loop(msg) printf(msg); \
    while(1) { \
    __asm__ __volatile__ ("hlt;"); \
    }

#endif
