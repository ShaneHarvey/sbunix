#include <sbunix/syscall.h>

/**
 * TODO: this
 *
 * @req: the requested amount of time to sleep
 * @rem: filled in will zeros if it is not NULL. Since we don't have signals
 * a process always sleep for the entire request
 */
int do_nanosleep(const struct timespec *req, struct timespec *rem) {
    return -ENOSYS;
}
