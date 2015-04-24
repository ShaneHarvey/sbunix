#ifndef _TIME_H
#define _TIME_H

#include <sys/types.h>

struct timespec {
    time_t  tv_sec; /* seconds */
    long  tv_nsec; /* nanoseconds */
};

struct itimerspec {
    struct timespec  it_interval;  /* Timer period */
    struct timespec  it_value;     /* Timer expiration */
};

int nanosleep(const struct timespec *req, struct timespec *rem);

#endif