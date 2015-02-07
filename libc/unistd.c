#include <stdlib.h>

/**
* todo: hook and call uname(2) to implement gethostname
*/
int gethostname(char *name, size_t len) {
    int hostfd;
    int b;

    if(len <= 0) {
        /* todo: errno = EINVAL; */
        return -1;
    }

    hostfd = open("/etc/hostname", O_RDONLY);
    if(hostfd < 0) {
        return -1;
    }

    b = read(hostfd, name, len);
    if(b < 0) {
        close(hostfd);
        return -1;
    }

    close(hostfd);
    return 1;
}