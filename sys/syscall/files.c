#include <sbunix/sbunix.h>
#include <sbunix/fs/tarfs.h>
#include <sbunix/fs/pipe.h>
#include <sbunix/sched.h>
#include <sbunix/syscall.h>
#include <sbunix/mm/vmm.h>
#include <sys/mman.h>

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
 * @addr:   start of new mapping (can be changed), if NULL kerenel choose start
 * @length: length of new mapping
 * @prot:   PROT_NONE or bitwise or of: PROT_EXEC, PROT_READ, PROT_WRITE
 * @flags:  MAP_ANONYMOUS no file, initially zero
 * @fd:     the file to mmap, if not MAP_ANONYMOUS
 * @offset: offset into file to start at (must be multiple of PAGE_SIZE)
 *
 * @return: the actual start of the new mapping (aligned to PAGE_SIZE)
 */
void *do_mmap(void *addr, size_t length, int prot, int flags, int fd,
              off_t offset) {
    uint64_t vm_prot = 0;
    struct file *filep;
    uint64_t mmap_start;
    int err, shared, private;

    shared = flags & MAP_SHARED;
    private = flags & MAP_PRIVATE;
    if((!shared && !private) || (shared && private))
        return (void*)-EINVAL;  /* Must contain exactly one */

    /* We won't support MAP_FIXED, or user choosing mmap location */
    if(length == 0 || flags & MAP_FIXED || addr != NULL)
        return (void*)-EINVAL;


    if(flags & MAP_ANONYMOUS) {
        /* Don't even look at fd */
        filep = NULL;
    } else {
        /* Get the user's open file */
        if(INVALID_FD(fd))
            return (void*)-EBADF;
        filep = curr_task->files[fd];
        if(!filep)
            return (void*)-EBADF;

        if(IS_ALIGNED(offset, PAGE_SIZE))
            return (void*)-EINVAL;  /* Offset not aligned to PAGE_SIZE */

        /*TODO: also EACCES if filep is not regular ! */
    }

    if(prot & PROT_WRITE && shared)
        return (void*)-EACCES;  /*  Writing into tarfs (our only fs) */

    if(prot & PROT_WRITE)
        vm_prot |= PFLAG_RW; /* Only need flag, users can always EXEC*/

    /* Find a region big enough */
    mmap_start = find_mmap_space(curr_task->mm, length);
    if(!mmap_start)
        return (void*)-ENOMEM;  /* No mmap space! */

    /* Finally! The call to mmap the area! */
    err = mmap_area(curr_task->mm, filep, offset, filep->f_size, vm_prot,
                    mmap_start, mmap_start + length);
    if(err)
        return (void*)(int64_t)err;

    return 0;
}

/**
 * Un-map a mmap'd area, it is not an error if the indicated range
 * does not contain any mapped pages.
 *
 * @addr:   returned from call to mmap
 * @length: non-zero size of region
 */
int do_munmap(void *addr, size_t length) {
    struct vm_area *vma;
    if(IS_ALIGNED((uint64_t)addr, PAGE_SIZE) || length == 0) {
        return -EINVAL;
    }  /* addr not aligned to PAGE_SIZE */

    vma = vma_find_region(curr_task->mm->vmas, (uint64_t)addr, length);
    if(!vma) {
        return 0;
    }
    /* TODO: call free_pagetbl_range_and_pages (only for munmap) */

    mm_remove_vma(curr_task->mm, vma);
    vma_destroy(vma);

    return 0;
}
