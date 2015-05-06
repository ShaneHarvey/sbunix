#include <sbunix/sbunix.h>
#include <sbunix/fs/tarfs.h>
#include <sbunix/fs/elf64.h>
#include <sbunix/mm/vmm.h>
#include <sbunix/gdt.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>
#include <sbunix/syscall.h>
#include <errno.h>

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

    if(oldfd < 0 || oldfd >= TASK_FILES_MAX ||
       newfd < 0 || newfd >= TASK_FILES_MAX)
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
