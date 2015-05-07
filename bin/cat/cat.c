#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>        /* open */
#include <errno.h>

/**
 * Dump all the contents of the open file fd to stdout
 *
 * @fd: a file open for reading
 * @return: exit code, 0 success
 */
int cat(int fd, char *filename) {
    char buf[4096];
    ssize_t nread;

    do {
        nread = read(fd, buf, sizeof(buf));
        if(nread < 0) {
            printf("cat: read error on %s: %s\n", filename, strerror(errno));
            return 1;
        }
        write(STDOUT_FILENO, buf, nread);
    } while (nread);

    return 0;
}

int main(int argc, char **argv, char **envp) {
    int err, fd, i;

    if(argc <= 1)
        err = cat(STDIN_FILENO, "stdin");

    for(i = 1; i < argc; i++) {
        fd = open(argv[i], O_RDONLY);
        if(fd < 0) {
            printf("cat: %s: %s\n", argv[i], strerror(errno));
            err = EXIT_FAILURE;
        } else {
            err = cat(fd, argv[i]);
        }
        close(fd);
    }
    return err;
}
