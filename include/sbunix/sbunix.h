#ifndef _SBUNIX_SBUNIX_H
#define _SBUNIX_SBUNIX_H

#include <sys/defs.h>

void printf(const char *fmt, ...);

#ifdef DEBUG
#   define debug(fmt, ...)   printf("DEBUG: %s:%s: " fmt, __FILE__, __FUNCTION__, ##__VA_ARGS__);
#else
#   define debug(S, ...)
#endif /* DEBUG */

#endif
