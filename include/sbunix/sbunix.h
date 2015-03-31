#ifndef _SBUNIX_SBUNIX_H
#define _SBUNIX_SBUNIX_H

#include <sys/defs.h>
#include <sbunix/asm.h>

#define kpanic(fmt, ...)     halt_loop("\n[PANIC] %s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#ifdef DEBUG
#   define debug(fmt, ...)   printf("[DB] %s():%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
/* #   define debug(fmt, ...)   printf("[DB] %s:%s: " fmt, __FILE__, __FUNCTION__, ##__VA_ARGS__) */
#else
#   define debug(S, ...)
#endif /* DEBUG */


void printf(const char *fmt, ...);

#endif
