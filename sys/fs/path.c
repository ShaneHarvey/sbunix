#include <sbunix/sbunix.h>
#include <sbunix/string.h>
#include <errno.h>


/**
 * Resolve a path with cwd
 *
 * @cwd: current directory
 * @path: relative or absolute path
 * @err: pointer to error indicator
 *
 * @return kmalloc'd pointer to the fully resolved path
 */
char *resolve_path(const char *cwd, const char *path, long *err) {
    if(!path) {
        *err = -EFAULT;
        return NULL;
    }
    *err = -ENOMEM;
    return NULL;
}
