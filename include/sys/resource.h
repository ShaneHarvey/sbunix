#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <sys/time.h>

struct rusage {
    /* at least the following members */
    struct timeval ru_utime;  /* User time used */
    struct timeval ru_stime;  /* System time used */

};

#endif