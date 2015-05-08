#include <sbunix/sbunix.h>
#include <sbunix/syscall.h>
#include <sbunix/string.h>
#include <sbunix/sched.h>
#include <sbunix/fs/tarfs.h>

/**
 * Just copy in the task's current directory
 */
long do_getcwd(char *buf, size_t size) {
    size_t cwd_len;

    if(!buf)
        return -EFAULT;

    cwd_len = strlen(curr_task->cwd);
    if(cwd_len + 1 > size)
        return -ERANGE;

    strcpy(buf, curr_task->cwd);
    return 0;
}

/**
 * Change the current working directory of the calling process to the
 * directory specified in path.
 *
 * @path: a relative or absolute path
 *
 * ERRORS
 * EFAULT        path points outside your accessible address space.
 * ELOOP         Too many symbolic links were encountered in resolving path.
 * ENAMETOOLONG  path is too long.
 * ENOENT        The file does not exist.
 * ENOMEM        Insufficient kernel memory was available.
 * ENOTDIR       A component of path is not a directory.
 */
long do_chdir(const char *path) {
    char *rpath;
    long err;

    rpath = resolve_path(curr_task->cwd, path, &err);
    if(!rpath)
        return err;

    if(strlen(rpath) > TASK_CWD_MAX) {
        err = -ENAMETOOLONG;
        goto out_rpath;
    }

    err = tarfs_isdir(rpath);
    if(err)
        goto out_rpath;

    strcpy(curr_task->cwd, rpath);
    return 0;

out_rpath:
    kfree(rpath);
    return err;
}
