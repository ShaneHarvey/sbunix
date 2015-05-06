#include <sbunix/syscall.h>

/**
 * TODO: this
 * If the length of the absolute pathname of the current working directory,
 * including the terminating null byte, exceeds size bytes, NULL is returned,
 * and errno is set to ERANGE; an application should check for this error, and
 * allocate a larger buffer if necessary.
 */
char *do_getcwd(char *buf, size_t size) {
    return (void *)-ENOSYS;
}

/**
 * TODO: this
 *
 * @path: a relative or absolute path
 */
int do_chdir(const char *path) {
    return -ENOSYS;
}
