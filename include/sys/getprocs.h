#ifndef SBUNIX_GETPROCS_H
#define SBUNIX_GETPROCS_H

#include <sys/types.h>

/* For getprocs(2) */
struct proc_struct {
    pid_t pid;
    char cmd[];
};

/**
 * The syscall to enable ps(1)
 * @procbuf: buffer into which proc_struct's will be stored
 * @length:  bytes in the buffer
 */
ssize_t getprocs(void *procbuf, size_t length);


#endif //SBUNIX_GETPROCS_H
