#include <sbunix/sbunix.h>
#include <sbunix/fs/tarfs.h>
#include <sbunix/fs/pipe.h>
#include <sbunix/sched.h>
#include <sbunix/syscall.h>

#define INVALID_FD(fd) ((fd) < 0 || (fd) >= TASK_FILES_MAX)

/**
 * Returns the next free file descriptor in the
 */
int next_fd(struct task_struct *task) {
    int i;

    for(i = 0; i < TASK_FILES_MAX; i++) {
        if(task->files[i] == NULL)
            return i;
    }
    return -EMFILE; /* no free files */
}

/*
 * File descriptor functions.
 * Nothing special here, just call the file's underlying function.
 */

/**
 * @mode: unused
 */
int do_open(const char *pathname, int flags, mode_t mode) {
    int newfd, err;
    struct file *newfilep;

    if(!pathname)
        return -EFAULT;

    /* pick a new fd index */
    newfd = next_fd(curr_task);
    if(newfd < 0)
        return newfd; /* too many files */
    /* TODO: Resolve pathname to an absolute path */
    newfilep = tarfs_open(pathname, flags, mode, &err);
    if(err)
        return err;
    /* success */
    curr_task->files[newfd] = newfilep;
    return newfd;
}

ssize_t do_read(int fd, void *buf, size_t count) {
    struct file *filep;

    if(INVALID_FD(fd))
        return -EBADF;
    filep = curr_task->files[fd];
    if(!filep)
        return -EBADF;
    return filep->f_op->read(filep, buf, count, &filep->f_pos);
}

ssize_t do_write(int fd, const void *buf, size_t count) {
    struct file *filep;

    if(INVALID_FD(fd))
        return -EBADF;
    filep = curr_task->files[fd];
    if(!filep)
        return -EBADF;
    return filep->f_op->write(filep, buf, count, &filep->f_pos);
}

off_t do_lseek(int fd, off_t offset, int whence) {
    struct file *filep;

    if(INVALID_FD(fd))
        return -EBADF;
    filep = curr_task->files[fd];
    if(!filep)
        return -EBADF;
    return filep->f_op->lseek(filep, offset, whence);
}

int do_close(int fd) {
    int rv;
    struct file *filep;

    if(INVALID_FD(fd))
        return -EBADF;
    filep = curr_task->files[fd];
    if(!filep)
        return -EBADF;
    rv = filep->f_op->close(filep);
    curr_task->files[fd] = NULL;
    return rv;
}

/**
 * Create a unidirectional data channel. pipefd[0] is the read end,
 * pipefd[1] in the write end.
 *
 * @pipefd: valid pointer to an array of two integers.
 */
int do_pipe(int *pipefd) {
    int rfd, wfd, err;

    if(!pipefd)
        return -EFAULT;

    /* Choose 2 open file descriptors */
    for(rfd = 0; rfd < TASK_FILES_MAX; rfd++)
        if(curr_task->files[rfd] == NULL)
            break;
    for(wfd = rfd + 1; wfd < TASK_FILES_MAX; wfd++)
        if(curr_task->files[wfd] == NULL)
            break;
    if(rfd >= TASK_FILES_MAX || wfd >= TASK_FILES_MAX)
        return -EMFILE;

    err = pipe_open(&curr_task->files[rfd], &curr_task->files[wfd]);
    if(err)
        return err;
    /* Update user fd's */
    pipefd[0] = rfd;
    pipefd[1] = wfd;
    return 0;
}

/**
 * Creates a copy of the specified file descriptor.
 *
 * @oldfd: file descriptor to copy
 * @return: newfd on success, -ERRNO on failure
 */
int do_dup(int oldfd) {
    int newfd;

    newfd = next_fd(curr_task); /* find a valid fd */
    if(newfd >= 0)
        newfd = do_dup2(oldfd, newfd); /* just use dup2 */
    return newfd;
}

/**
 * Copies oldfd to newfd, closing newfd if necessary
 *
 * If oldfd is not a valid file descriptor, then the cal  fails, and
 * newfd is not closed.
 *
 * If oldfd is a valid file descriptor, and newfd has the same value as
 * oldfd, thendup2() does nothing, and returns newfd.
 *
 * @oldfd: source file descriptor to copy
 * @newfd: destination file descriptor
 * @return: newfd on success, -ERRNO on failure
 */
int do_dup2(int oldfd, int newfd) {
    struct file *newfilep, *oldfilep;

    if(INVALID_FD(oldfd) || INVALID_FD(newfd))
        return -EBADF; /* invalid fd index */

    oldfilep = curr_task->files[oldfd];
    if(!oldfilep)
        return -EBADF; /* oldfd is not an opened file */

    if(oldfd == newfd)
        return newfd;  /* same fd index */

    newfilep = curr_task->files[newfd];
    if(newfilep)
        newfilep->f_op->close(newfilep); /* close new fd */
    oldfilep->f_count++;
    curr_task->files[newfd] = oldfilep;
    return newfd;
}

/**
 * TODO: this
 *
 * @fd:  file descriptor of a opened directory file
 * @dirp: pointer to count bytes
 * @count: the number of bytes in dirp
 */
int do_getdents(unsigned int fd, struct dirent *dirp, unsigned int count) {
    if(!dirp)
        return -EFAULT;

    if(INVALID_FD(fd))
        return -EBADF;

    return -ENOSYS;
}


/**
 * TODO: this
 */
void *do_mmap(void *addr, size_t length, int prot, int flags, int fd,
              off_t offset) {
    return (void *)-ENOSYS;
}

/**
 * TODO: this
 */
int do_munmap(void *addr, size_t length) {
    return -ENOSYS;
}
