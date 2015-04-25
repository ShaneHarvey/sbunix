#ifndef _SBUNIX_TIME_H
#define _SBUNIX_TIME_H

typedef long int time_t;

struct timespec {
    time_t  tv_sec; /* seconds */
    long  tv_nsec;  /* nanoseconds */
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

extern struct timespec unix_time;     /* real (UNIX) time */


/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines were long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
static inline unsigned long mktime(unsigned int year, unsigned int mon,
                                   unsigned int day, unsigned int hour, unsigned int min, unsigned int sec) {
    if (0 >= (int) (mon -= 2)) {    /* 1..12 -> 11,12,1..10 */
        mon += 12;              /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return ((((unsigned long) (year/4 - year/100 + year/400 + 367*mon/12 + day)
              + year*365 - 719499
             )*24 + hour /* now have hours */
            )*60 + min /* now have minutes */
           )*60 + sec; /* finally seconds */
}

void init_unix_time(void);

#endif //_SBUNIX_TIME_H
