#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>


struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
    /* length of d_name == (d_reclen - 2 -
       offsetof(struct linux_dirent, d_name)) */
    /*
    char           pad;       // Zero padding byte
    char           d_type;    // File type (only since Linux
                              // 2.6.4); offset is (d_reclen - 1)
    */
};

/**
* The opendir() function opens a directory stream corresponding to the directory name, and
* returns a pointer to the directory stream.  The stream is positioned at the first  entry
* in the directory.
*/
DIR *opendir(const char *name) {
    int fd;
    DIR *dirp;
    if(!name || name[0] == '\0') {
        errno = ENOENT;
        return NULL;
    }
    /* TODO could add O_CLOEXEC */
    fd = open(name, O_RDONLY | O_DIRECTORY);
    if(fd < 0) {
        return NULL;
    }
    dirp = malloc(sizeof(DIR));
    if(!dirp) {
        return NULL;
    }
    dirp->buf = malloc(BUFSIZ); /* buf for dirents */
    if(!dirp->buf) {
        free(dirp);
        return NULL;
    }
    dirp->space = BUFSIZ;
    dirp->offset = dirp->size = 0;
    dirp->filepos = 0;
    return dirp;
}

/**
* The readdir() function returns a pointer to a dirent  structure  representing  the  next
* directory entry in the directory stream pointed to by dirp.  It returns NULL on reaching
* the end of the directory stream or if an error occurred.
*/
struct dirent *readdir(DIR *dirp) {
    struct linux_dirent *ldp = NULL;
    struct dirent *dp = NULL;
    unsigned char d_type;
    int bytesread = 0;

    do {
        if(dirp->offset >= dirp->size) {
            /* Must call getdents */
            bytesread = getdents((uint)dirp->fd, (struct dirent*)dirp->buf, (uint)dirp->space);
            if(bytesread <= 0) {
                return NULL; /* End of dir, or error (errno set) */
            }
            dirp->size = (size_t)bytesread;
            dirp->offset = 0;
        }
        ldp = (struct linux_dirent*)(dirp->buf + dirp->offset);
        dirp->offset += ldp->d_reclen;

    } while(ldp->d_ino == 0); /* Means file dirent was deleted */

    dirp->filepos = (off_t)ldp->d_off;
    dp = (struct dirent *)ldp;

    /* convert linux_dirent to dirent */
    d_type = *((unsigned char*)ldp + ldp->d_reclen);
    memmove(dp->d_name, ldp->d_name, (ldp->d_reclen - 2 - 2*sizeof(long) - sizeof(short)));
    dp->d_type = d_type;

    return dp;
}

/**
* The  closedir() function closes the directory stream associated with dirp.  A successful
* call to closedir() also closes the underlying file descriptor associated with dirp.  The
* directory stream descriptor dirp is not available after this call.
*/
int closedir(DIR *dirp) {
    int fd;
    if(!dirp || !dirp->buf) {
        errno = EINVAL;
        return -1;
    }
    fd = dirp->fd;
    free(dirp->buf);
    free(dirp);
    return close(fd);
}
