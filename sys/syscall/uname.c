#include <sbunix/syscall.h>
#include <sbunix/string.h>

int do_uname(struct utsname *buf) {
    if(!buf)
        return -EFAULT;
    strcpy(buf->sysname, "SBUnix");
    strcpy(buf->nodename, "(none)");
    strcpy(buf->release, "1.0");
    strcpy(buf->version, "Yesterday");
    strcpy(buf->machine, "x86_64");
    strcpy(buf->domainname, "(none)");
    return 0;
}
