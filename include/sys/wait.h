#ifndef _WAIT_H
#define _WAIT_H

#include <sys/types.h>

/* Maybe we should move protos of wait* to here */

pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage);

#endif
