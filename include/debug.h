#ifndef _DEBUG_H
#define _DEBUG_H 1

#include <stdio.h> /* for printf/fprintf */

#if defined(COLOR)
#  define _RED     "\x1b[31m"
#  define _GREEN   "\x1b[32m"
#  define _YELLOW  "\x1b[33m"
#  define _BLUE    "\x1b[34m"
#  define _PURPLE  "\x1b[35m"
#  define _RESET   "\x1b[0m"
#else
#  define _RED
#  define _GREEN
#  define _YELLOW
#  define _BLUE
#  define _PURPLE
#  define _RESET
#endif


#if defined(DEBUG)

#  define debug(fmt, ...)   printf("DEBUG: %s:%s:%d: " fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  define debugy(fmt, ...)  printf(_YELLOW "DEBUGY: %s:%s:%d: " fmt _RESET, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#  define warn(fmt, ...)    printf(_YELLOW "WARN: %s:%s:%d: " fmt _RESET, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  define info(fmt, ...)    printf(_BLUE "INFO: %s:%s:%d: " fmt _RESET, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  define success(fmt, ...) printf(_GREEN "SUCCESS: %s:%s:%d: " fmt _RESET, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#  define error(fmt, ...)   printf(_RED "ERROR: %s:%s:%d: " fmt _RESET, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#else

#  define debug(fmt, ...)
#  define debugy(fmt, ...)

#  define warn(fmt, ...)    printf(_YELLOW "WARN: " fmt _RESET, ##__VA_ARGS__)
#  define info(fmt, ...)    printf(_BLUE "INFO: " fmt _RESET, ##__VA_ARGS__)
#  define success(fmt, ...) printf(_GREEN "SUCCESS: " fmt _RESET, ##__VA_ARGS__)
#  define error(fmt, ...)   printf(_RED "ERROR: " fmt _RESET, ##__VA_ARGS__)

#endif /* DEBUG macros */

#endif /* _DEBUG_H */
