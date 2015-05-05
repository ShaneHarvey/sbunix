#include "test.h"

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
    printk("END TEST TERMINAL\n");

    kill_curr_task();
}
