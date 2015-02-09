#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

int gethostname(char *name, size_t len) {
    struct utsname buf;
    int rv;
    if(name == NULL) {
        /* todo: errno = EFAULT; */
        return -1;
    }
    if(len <= 0) {
        /* todo: errno = EINVAL; */
        return -1;
    }

    rv = uname(&buf);
    if(rv < 0) {
        return -1;
    }
    if(len < strlen(buf.nodename)) {
        /* todo: errno = ENAMETOOLONG; */
        return -1;
    }
    strcpy(name, buf.nodename);
    return 1;
}