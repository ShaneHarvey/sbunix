#include <sbunix/sbunix.h>
#include <sbunix/console.h>
#include <sbunix/fs/terminal.h>
#include <sbunix/sched.h>
#include <errno.h>
#include <sbunix/string.h>

/**
 * STDIN, STDOUT, STDERR are all going to be backed by the same terminal.
 */

#define TERM_BUFSIZE 4096
struct terminal_buf {
    // TODO: controlling pid? for job control? tcsetpgrp(2)
    int start;                       /* Read head of the buffer  (first occupied cell) */
    int end;                         /* Write head of the buffer (next empty cell) */
    char full;                       /* If the buffer is full */
    char echo;                       /* If local echo is enabled */
    volatile char delims;            /* number of line delimiters in the buffer */
    int  backspace;                  /* number backspaces we can take */
    unsigned char buf[TERM_BUFSIZE]; /* Holds buffered characters */
};

struct terminal_buf term = {
        .start     = 0,
        .end       = 0,
        .full      = 0,
        .echo      = 1,
        .delims    = 0,
        .backspace = 0,
        .buf       = {0}
};

/* File hooks for standard terminal input */
struct file_ops term_ops = {
        .lseek = term_lseek,
        .read = term_read,
        .write = term_write,
//    .readdir = term_readdir,
        .close = term_close
};

/* There is only one terminal ever for the system */
struct file term_file = {
        .f_op = &term_ops,
        .f_count = 1,
        .f_flags = 0,
        .f_pos = 0,
        .f_size = 0,
        .f_error = 0,
        .private_data = &term
};

/**
 * Reset the terminal to it's default state
 */
void term_reset(void) {
    term.start     = 0;
    term.end       = 0;
    term.full      = 0;
    term.echo      = 1;
    term.delims    = 0;
    term.backspace = 0;
}

/**
 * Helper function simply pushes the character onto the buffer and echo
 */
static void term_push(unsigned char c) {
    /* Add character */
    term.buf[term.end] = c;
    term.end = (term.end + 1) % TERM_BUFSIZE;
    if(term.end == term.start) {
        term.full = 1;
        term.delims = 1;
    }
}

/**
 * Helper pops the first character from the buffer
 *
 * @tb:  a valid terminal_buf
 *
 * @return the first character or -1 if there are none to read
 */
static int term_popfirst(struct terminal_buf *tb) {
    int c;

    if(tb->start == tb->end && !tb->full)
        return -1;

    c = tb->buf[tb->start];
    tb->start = (tb->start + 1) % TERM_BUFSIZE;
    term.full = 0;
    if(c == EOT) {
        tb->delims--;
        return -1;
    } else if(c == '\n') {
        tb->delims--;
    }
    return c;
}

/**
 * Pop and return the last character to be added from terminal buffer
 */
static int term_poplast(void) {
    if(term.backspace == 0) {
        return -1;
    }
    term.backspace--;
    /* Safe version of end--; */
    term.end = (term.end + (TERM_BUFSIZE - 1)) % TERM_BUFSIZE;
    term.full = 0;
    return term.buf[term.end];
}

/**
 * Append a character to the terminal input buffer
 * NOTE: interrupts are disabled here
 *
 * @c:  Character to enqueue
 */
void term_putch(unsigned char c) {
    /* handle backspace here */
    if(c == '\b') {
        if(term.backspace == 0)
            return;
        term_poplast();
        if(term.echo) {
            putch('\b');
            putch(' ');
            putch('\b');
            move_csr();
        }
        return;
    }
    if(term.full) {
        /* print a bell to notify that we lost a character */
//        putch('\a');
        task_unblock_foreground();
        return;
    }

    if(c == EOT || c == '\n') {
        term.delims++;
        term.backspace = 0;
        task_unblock_foreground();
    } else {
        /* TODO: handle other special characters here */
        term.backspace++;
    }
    /* Add c to the buffer */
    term_push(c);
    /* Local echo, print the last char and move the cursor */
    if(term.echo && c && c != EOT) {
        putch(c);
        move_csr();
    }
    if(term.full) {
        task_unblock_foreground();
    }
}

/**
 * Cannot seek on the terminal
 */
off_t term_lseek(struct file *fp, off_t offset, int whence) {
    return -ESPIPE; /* illegal seek */
}

/**
 * Terminal read
 *
 * Canonical Mode:
 * Input is made available line by line.  An input line is available when one of the line
 * delimiters is typed (NL, EOF at the start of line).  Except in the  case
 * of EOF, the line delimiter is included in the buffer returned by read(2).
 *
 * @fp:  pointer to a file struct backed by a terminal
 * @buf: user buffer to read into
 * @count: maximum number of bytes to read
 * @offset: unused, terminal always reads from the beginning of its buffer
 */
ssize_t term_read(struct file *fp, char *buf, size_t count, off_t *offset) {
    struct terminal_buf *tb;
    ssize_t num_read;
    int c;

    /* Error checking */
    if(!fp || !buf)
        return -EINVAL;
    tb = (struct terminal_buf *)fp->private_data;
    if(!tb)
        return -EINVAL;
    /* Do read */
    if(count == 0)
        return 0;
    /* Is this task controlling the terminal */
    if(!curr_task->foreground)
        return -EIO; /* should send SIGTTIN as well, but don't have signals */

    if(tb->delims == 0) {
        /* Block as a line has not been buffered yet */
        task_block();
    }
//    if(tb->delims == 0) {
//        kpanic("Unblocked but still no input!!!\n");
//    }
    /* Unblocked! We can read until delim or count bytes are consumed */
    num_read = 0;
    while(count--) {
        c = term_popfirst(tb);
        if(c < 0)
            break;
        buf[num_read++] = (char)c;
    }
    return num_read;
}

/**
 * Write to the terminal backed file fp, this writes directly to the console.
 *
 * @fp:  pointer to a file struct backed by a terminal
 * @buf: user buffer to output to the console
 * @count: number of bytes to output
 * @offset: unused, terminal always writes to the current cursor
 */
ssize_t term_write(struct file *fp, const char *buf, size_t count,
                   off_t *offset) {
    struct terminal_buf *tb;
    ssize_t num_written;

    /* Error checking */
    if(!fp || !buf)
        return -EINVAL;
    tb = (struct terminal_buf *)fp->private_data;
    if(!tb)
        return -EINVAL;
    /* Print buffer to the console */
    num_written = 0;
    while(count--) {
        putch(*buf++);
        num_written++;
    }
    move_csr();
    return num_written;
}

/**
 * Opens a new terminal
 */
struct file *term_open(void) {
    term_file.f_count++;
    return &term_file;
}

/**
 * Close a file to free any information related to the file.
 * Terminal simply returns success
 */
int term_close(struct file *fp) {
    if(!fp)
        kpanic("file is NULL!!!\n");
    fp->f_count--;
    return 0;
}


/**
 * Simple test of input/output on terminal file.
 */
void test_terminal(void) {
    struct file *stdin, *stdout;
    char buf[100] = {0};
    ssize_t nread, nbytes;
    stdin = term_open();
    if(!stdin)
        return;
    stdout = term_open();
    if(!stdout)
        goto cleanup_stdin;
    /* Read from stdin, this should block the current process */
    while(1) {
        nread = stdin->f_op->read(stdin, buf, 99, NULL);
        if(nread < 0) {
            printk("ERROR: %s\n", strerror(-(int)nread));
            goto cleanup_stdout;
        } else if(nread == 0) {
            printk("READ EOF.\n");
            break;
        }
        /* Write to stdout */
        nbytes = stdout->f_op->write(stdout, "ECHO: ", 6, NULL);
        if(nbytes < 6) {
            printk("ERROR: nbytes written = %d\n", (int)nread);
            goto cleanup_stdout;
        }
        nbytes = stdout->f_op->write(stdout, buf, (size_t)nread, NULL);
        if(nbytes < nread) {
            printk("ERROR: nbytes written = %d\n", (int)nread);
            goto cleanup_stdout;
        }
    }
    /* Close files */
cleanup_stdout:
    stdout->f_op->close(stdout);
cleanup_stdin:
    stdin->f_op->close(stdin);
    kpanic("END TEST TERMINAL\n");
}
