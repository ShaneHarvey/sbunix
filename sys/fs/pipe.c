#include <sbunix/fs/pipe.h>
#include <errno.h>

/**
 * The array pipefd is used to return two file descriptors referring to the
 * ends of the pipe. pipefd[0] refers to the read end of the pipe. pipefd[1]
 * refers to the write end of the pipe. Data written to the write end of the
 * pipe is buffered by the kernel until it is read from the read end of the
 * pipe.
 */

#define PIPE_BUFSIZE 4000
struct pipe_buf {
    int start;                       /* Read head of the buffer  (first occupied cell) */
    int end;                         /* Write head of the buffer (next empty cell) */
    char full;                       /* If the buffer is full */
    char read_closed;                /* All the read ends have been closed */
    char write_closed;               /* All the write ends have been closed */
    unsigned char buf[PIPE_BUFSIZE]; /* Holds buffered data */
};

/* File hooks for read end of a pipe */
struct file_ops read_end_ops = {
        .lseek = pipe_lseek,
        .read = read_end_read,
        .write = read_end_write,
//    .readdir = pipe_readdir,
        .close = read_end_close
};
/* File hooks for read end of a pipe */
struct file_ops write_end_ops = {
        .lseek = pipe_lseek,
        .read = write_end_read,
        .write = write_end_write,
//    .readdir = pipe_readdir,
        .close = write_end_close
};

/**
 * Cannot seek on a pipe
 */
off_t pipe_lseek(struct file *fp, off_t offset, int whence) {
    return -ESPIPE;
}

/* Read end operations */
ssize_t read_end_read(struct file *fp, char *buf, size_t count,
                      off_t *offset) {
    return -ENOSYS;
}

/**
 * Cannot write to the read end of a pipe
 */
ssize_t read_end_write(struct file *fp, const char *buf, size_t count,
                       off_t *offset) {
    return -EBADF;
}

int read_end_close(struct file *fp) {
    return -ENOSYS;
}

/* Write end operations */

/**
 * Cannot read from the write end of a pipe
 */
ssize_t write_end_read(struct file *fp, char *buf, size_t count,
                       off_t *offset) {
    return -EBADF;
}

ssize_t write_end_write(struct file *fp, const char *buf, size_t count,
                        off_t *offset) {
    return -ENOSYS;
}
int write_end_close(struct file *fp) {
    return -ENOSYS;
}
