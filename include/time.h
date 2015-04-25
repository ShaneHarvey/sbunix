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

struct tm {
    int tm_sec;
    /* the number of minutes after the hour, in the range 0 to 59*/
    int tm_min;
    /* the number of hours past midnight, in the range 0 to 23 */
    int tm_hour;
    /* the day of the month, in the range 1 to 31 */
    int tm_mday;
    /* the number of months since January, in the range 0 to 11 */
    int tm_mon;
    /* the number of years since 1900 */
    long tm_year;
    /* the number of days since Sunday, in the range 0 to 6 */
    int tm_wday;
    /* the number of days since January 1, in the range 0 to 365 */
    int tm_yday;
};

int nanosleep(const struct timespec *req, struct timespec *rem);

#endif