#include <sbunix/sbunix.h>
#include <sbunix/string.h>
#include <sbunix/fs/tarfs.h>
#include <errno.h>

/* "types" of files in a path */
enum {
    F_NONE,   /* ""            */
    F_SLASH,  /* "/"           */
    F_DOT,    /* "." or "./"   */
    F_DOTDOT, /* ".." or "../" */
    F_DIR,    /* "foo/"         */
    F_FILE,   /* "bar"         */
};

/**
 * Return the type of the next file in the path.
 *
 * @path: pointer to the first character in this file
 */
static int next_file(char *path) {
    char c = *path;
    if(c == '\0')
        return F_NONE;
    if(c == '/')
        return F_SLASH;
    if(c == '.') {
        if(path[1] == '\0' || path[1] == '/')
            return F_DOT;
        else if(path[1] == '.') {
            if(path[2] == '\0' || path[2] == '/')
                return F_DOTDOT;
        }
    }
    if(strchr(path, '/'))
        return F_DIR;
    return F_FILE;
}

/**
 * Helper function that walks the path and resolves "..", ".", and "//"
 *
 * @rpath: the absolute path to validate
 * @vaild: valid points to the end of the valid path so far, or NULL
 *
 * @return 0 on success
 */
static long _resolve_path(char *rpath, char *valid) {
    char *cur;
    long err;

    if(!rpath || *rpath != '/')
        kpanic("BUG: rpath not initialized correctly!\n");

    if(!valid)
        valid = rpath;

    cur = valid + 1; /* cur points to start of the next file in path */
    while(*cur) {

        /* valid |
         *       V
         * "/root/..[/unknown...]"
         *        ^
         *    cur |
         */
        switch(next_file(cur)) {
            case F_NONE:
                /* Revert vaild */
                return 0; /* DONE! */
            case F_SLASH: /* / and . are the same case */
            case F_DOT:   /* just increment to next file */
                *cur++ = '\0';
                continue;
            case F_DOTDOT:
                /* need to go back one dir (until previous '/') */
                while(valid > rpath && *--valid != '/'); /* nothing */;
                valid[1] = '\0';
                /* cur skips 2 spaces */
                *cur++ = '\0';
                *cur++ = '\0';
                break;
            case F_DIR:
                if(valid + 1 == cur) {
                    /* valid and cur are contiguous */
                    valid = strchr(cur, '/');
                    cur = valid + 1;
                } else {
                    /* need to copy cur dir to valid */
                    /*  |valid   |cur
                     *  V        V
                     * "/root/../foo/[unknown...]"
                     *
                     *      |valid   |cur
                     *      V        V
                     * "/foo//../foo/[unknown...]"
                     */
                    while((*++valid = *cur++) != '/'); /* nothing */;
                }
                /* Validate the new path */
                *valid = '\0';
                err = tarfs_isdir(rpath);
                if(err)
                    return err;
                *valid = '/';
                break;
            case F_FILE:
                /* reached the end of the path! */
                if(valid + 1 == cur) {
                    /* valid and cur are contiguous */
                    valid += strlen(cur) + 1;
                } else {
                    /* need to copy cur dir to valid */
                    /*  |valid   |cur
                     *  V        V
                     * "/root/../foo/[unknown...]"
                     *
                     *      |valid   |cur
                     *      V        V
                     * "/foo//../foo/[unknown...]"
                     */
                    while((*++valid = *cur++) != '\0'); /* nothing */;
                }
                return tarfs_isfile(rpath);
            default:
                kpanic("Impossible default case!\n");
        }
    }
    /* Success! valid path to a file/directory */
    if(valid != rpath)
        *valid = '\0'; /* remove the trailing '/' */
    return 0;
}

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
    char *rpath, *valid;
    size_t len;

    if(!path || *path == '\0') {
        *err = -EFAULT;
        return NULL;
    }
    rpath = kmalloc(sizeof(PAGE_SIZE));
    if(!rpath) {
        *err = -ENOMEM;
        return NULL;
    }
    if(*path == '/') {
        strncpy(rpath, path, PAGE_SIZE - 1);
        valid = NULL;
    } else {
        /* TODO: path must be no longer than PAGE_SIZE - TASK_CWD_MAX - 2 */
        /* concat cwd with path */
        len = strlen(cwd);
        strcpy(rpath, cwd);
        /* Add a '/' in between if we need to */
        if(rpath[len - 1] != '/')
            rpath[len++] = '/';
        valid = rpath + len - 1; /* Points to the end of cwd */
        strncpy(rpath + len, path, PAGE_SIZE - len - 1);
    }
    rpath[PAGE_SIZE - 1] = '\0'; /* null term path */

    /* now we can resolve "..", ".", and "//" */
    *err = _resolve_path(rpath, valid);
    if(*err)
        goto out_rpath;


    return rpath;
out_rpath:
    kfree(rpath);
    return NULL;
}
