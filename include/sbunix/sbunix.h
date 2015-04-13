#ifndef _SBUNIX_SBUNIX_H
#define _SBUNIX_SBUNIX_H

#include <sys/defs.h>
#include <sbunix/asm.h>
#include <sbunix/mm/page_alloc.h>

#define kpanic(fmt, ...)     halt_loop("\n[PANIC] %s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#ifdef DEBUG
#   define debug(fmt, ...)   printf("[DB] %s():%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
/* #   define debug(fmt, ...)   printf("[DB] %s:%s: " fmt, __FILE__, __FUNCTION__, ##__VA_ARGS__) */
#else
#   define debug(S, ...)
#endif /* DEBUG */

extern uint64_t virt_base;

static __inline__ uint64_t kvirt_to_phys(uint64_t virt_addr) {
    return virt_addr - virt_base;
}

static __inline__ uint64_t kphys_to_virt(uint64_t phys_addr) {
    return phys_addr + virt_base;
}

void printf(const char *fmt, ...);

#endif
