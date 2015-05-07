#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <errno.h>

int gethostname(char *name, size_t len) {
    struct utsname buf;
    int rv;
    if(name == NULL) {
        errno = EFAULT;
        return -1;
    }
    if(len <= 0) {
        errno = EINVAL;
        return -1;
    }

    rv = uname(&buf);
    if(rv < 0) {
        return -1;
    }
    if(len < strlen(buf.sysname)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(name, buf.sysname);
    return 0;
}
