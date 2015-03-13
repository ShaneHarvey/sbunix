#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/resource.h>

/* Maybe we should move protos of wait* to here */

pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage);

#endif
