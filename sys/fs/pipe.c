#include <sbunix/fs/pipe.h>
#include <sbunix/sbunix.h>
#include <errno.h>
#include <sbunix/string.h>

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
/**
 * Pipe read.
 * If the pipe is empty, read will block until data is available OR
 * the write end is closed.
 * If the pipe has <count bytes, all bytes are consumed.
 * If the pipe is full, count bytes are consumed and read will unblock
 * any tasks blocking on a write.
 *
 * @fp:  pointer to a file struct backed by a read-end pipe
 * @buf: user buffer to read into
 * @count: maximum number of bytes to read
 * @offset: unused, always reads from the start of the pipe
 */
ssize_t read_end_read(struct file *fp, char *buf, size_t count,
                      off_t *offset) {
    struct pipe_buf *pipe;
    ssize_t num_read;
    int was_full;
    if(!fp)
        kpanic("file is NULL!");
    if(!buf)
        kpanic("user buffer is NULL!");
    pipe = (struct pipe_buf *)fp->private_data;
    if(!pipe)
        kpanic("pipe in file is NULL!");

    if(count == 0)
        return 0;

    /* Pipe is empty, either return EOF or block */
    while(pipe->start == pipe->end && !pipe->full) {
        if(pipe->write_closed) {
            return 0; /* Read the EOF */
        } else {
            /* TODO: block until able to write again */
            //block_task();
        }
    }
    /* pipe has data to read */
    was_full = pipe->full;
    num_read = 0;
    do {
        *buf++ = pipe->buf[pipe->start];
        pipe->start = (pipe->start + 1) % PIPE_BUFSIZE;
        pipe->full = 0;
        num_read++;
        /* read either count bytes OR until pipe is empty */
    } while(num_read < count && pipe->start != pipe->end);

    if(was_full) {
        /* TODO: unblock any task that may have been waiting to write */
    }
    return num_read;
}

/**
 * Cannot write to the read end of a pipe
 */
ssize_t read_end_write(struct file *fp, const char *buf, size_t count,
                       off_t *offset) {
    return -EBADF;
}

/**
 * Close the read end of the pipe referenced by file fp
 *
 * @fp: A read-end pipe file
 */
int read_end_close(struct file *fp) {
    if(!fp)
        kpanic("file is NULL!");
    fp->f_count--;
    if(fp->f_count == 0) {
        struct pipe_buf *pipe;
        pipe = (struct pipe_buf *)fp->private_data;
        if(!pipe)
            kpanic("pipe in file is NULL!");
        pipe->read_closed = 1;
        if(pipe->write_closed) {
            /* pipe has no more references to it */
            kfree(pipe);
            fp->private_data = NULL;
        } else {
            /* TODO: Unblock any task blocking on a write for THIS pipe */
            //wake_task();
        }
        kfree(fp);
    }
    return 0;
}

/* Write end operations */

/**
 * Cannot read from the write end of a pipe
 */
ssize_t write_end_read(struct file *fp, char *buf, size_t count,
                       off_t *offset) {
    return -EBADF;
}

/**
 * Pipe write
 *
 * @fp:  pointer to a file struct backed by the write end of a pipe
 * @buf: user buffer to output to the console
 * @count: number of bytes to output
 * @offset: unused, always writes to the end of the pipe
 */
ssize_t write_end_write(struct file *fp, const char *buf, size_t count,
                        off_t *offset) {
    struct pipe_buf *pipe;
    ssize_t num_written;
    if(!fp)
        kpanic("file is NULL!");
    if(!buf)
        kpanic("user buffer is NULL!");
    pipe = (struct pipe_buf *)fp->private_data;
    if(!pipe)
        kpanic("pipe in file is NULL!");

    /* Writing to a pipe that is closed is an error */
    if(pipe->read_closed)
        return -EPIPE;

    num_written = 0;
    while(num_written < count) {
        while(pipe->full) {
            /* TODO: First, unblock any task blocking on a read for THIS pipe */
            //wake_task();
            /* TODO: Then, block until someone wakes you up */
            //block_task();
        }
        /* pipe may have been closed while waiting */
        if(pipe->read_closed)
            break;
        /* Pipe should have room to write */
        pipe->buf[pipe->end] = (unsigned char)*buf++;
        pipe->end = (pipe->end + 1) % PIPE_BUFSIZE;
        pipe->full = (pipe->end == pipe->start);
        num_written++;
    }
    return num_written;
}

/**
 * Close the write end of the pipe referenced by file fp
 *
 * @fp: A write-end pipe file
 */
int write_end_close(struct file *fp) {
    if(!fp)
        kpanic("file is NULL!");
    fp->f_count--;
    if(fp->f_count == 0) {
        struct pipe_buf *pipe;
        pipe = (struct pipe_buf *)fp->private_data;
        if(!pipe)
            kpanic("pipe in file is NULL!");
        pipe->write_closed = 1;
        if(pipe->read_closed) {
            /* pipe has no more references to it */
            kfree(pipe);
            fp->private_data = NULL;
        } else {
            /* TODO: Unblock any task blocking on a read for THIS pipe */
            //wake_task();
        }
        kfree(fp);
    }
    return 0;
}

/**
 * Opens a pipe and initializes read_end and write_end
 *
 * @return 0 on success, <0 on error
 */
int pipe_open(struct file **read_end, struct file **write_end) {
    struct file *new_read, *new_write;
    struct pipe_buf *new_buf;

    /* Allocate space for pipe and files */
    new_read = kmalloc(sizeof(struct file));
    if(!new_read)
        goto out_nomem;
    new_write = kmalloc(sizeof(struct file));
    if(!new_write)
        goto out_read;
    new_buf = kmalloc(sizeof(struct pipe_buf));
    if(!new_buf)
        goto out_write;

    /* Init pipe buffer */
    memset(new_buf, 0, sizeof(struct pipe_buf));
    /* Init read end */
    new_read->f_op = &read_end_ops;
    new_read->f_count = 1;
    new_read->f_flags = 0;
    new_read->f_pos = 0;
    new_read->f_size = 0;
    new_read->f_error = 0;
    new_read->private_data = new_buf;
    /* Init write end */
    new_write->f_op = &write_end_ops;
    new_write->f_count = 1;
    new_write->f_flags = 0;
    new_write->f_pos = 0;
    new_write->f_size = 0;
    new_write->f_error = 0;
    new_write->private_data = new_buf;
    return 0;
out_write:
    kfree(new_write);
out_read:
    kfree(new_read);
out_nomem:
    return -ENOMEM;
}
