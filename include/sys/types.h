#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <sys/defs.h>

typedef int64_t ssize_t;
typedef unsigned long int size_t;
typedef unsigned int uint;
typedef int pid_t;

typedef long long off_t;
typedef unsigned long ino_t;

typedef long int time_t;
typedef long int suseconds_t;

typedef void * userptr_t;

typedef uint32_t mode_t;
#endif