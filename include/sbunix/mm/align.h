#ifndef _SBUNIX_MM_ALIGN_H
#define _SBUNIX_MM_ALIGN_H

/* Used by ALIGN_DOWN */
#define ALIGN_MASK(x, mask)     ( ((x) + (mask)) & ~(mask) )

/* Rounds down to multiple of almt (alignment), must power of 2 */
#define ALIGN_DOWN(ptr, almt)   ( (ptr) & ~(((__typeof__(ptr))(almt) - 1)) )

/* Same as above but rounds up. */
#define ALIGN_UP(ptr, almt)   ALIGN_MASK(ptr, ( ((__typeof__(ptr))(almt) - 1) ))

#define IS_ALIGNED(ptr, almt) (ptr & ((__typeof__(ptr))(almt) - 1))

#endif
