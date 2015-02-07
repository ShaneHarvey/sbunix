#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <sys/types.h>

struct timeval {
    /* at least */
    time_t      tv_sec;  /* seconds */
    suseconds_t tv_usec; /* microseconds */
};

struct itimerval {
    /* at least */
    struct timeval it_interval; /* Timer interval */
    struct timeval it_value;    /* Current value */
};

#endif