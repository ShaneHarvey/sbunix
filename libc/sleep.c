#include <unistd.h>
#include <time.h>
#include <syscall.h>

unsigned int sleep(unsigned int seconds) {
    const struct timespec req = { .tv_sec = seconds, .tv_nsec = 0};
    struct timespec rem = { .tv_sec = 0, .tv_nsec = 0};
    int rv;

    rv = nanosleep(&req, &rem);

    if(rv == 0) {
        return 0;
    } else {
        return (unsigned int)rem.tv_sec;
    }

}
