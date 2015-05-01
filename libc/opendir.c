#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>


/**
* The opendir() function opens a directory stream corresponding to the directory name, and
* returns a pointer to the directory stream.  The stream is positioned at the first  entry
* in the directory.
*/
void *opendir(const char *name) {
    int fd;
    struct dirstream *dirp;
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
    dirp->fd = fd;
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
struct dirent *readdir(void *dirp) {
    struct linux_dirent *ldp = NULL;
    struct dirent *dp = NULL;
    struct dirstream *dir = dirp;
    unsigned char d_type;
    int bytesread = 0;

    do {
        if(dir->offset >= dir->size) {
            /* Must call getdents */
            bytesread = getdents((uint) dir->fd, (struct dirent*) dir->buf, (uint) dir->space);
            if(bytesread <= 0) {
                return NULL; /* End of dir, or error (errno set) */
            }
            dir->size = (size_t)bytesread;
            dir->offset = 0;
        }
        ldp = (struct linux_dirent*)(dir->buf + dir->offset);
        dir->offset += ldp->d_reclen;

    } while(ldp->d_ino == 0); /* Means file dirent was deleted */

    dir->filepos = (off_t)ldp->d_off;
    dp = (struct dirent *)ldp;

    /* convert linux_dirent to dirent */
    d_type = *((unsigned char*)ldp + ldp->d_reclen - 1); /* save */
    memmove(dp->d_name, ldp->d_name, LDIR_NAMELEN(ldp) + 1); /* shift down 1 */
    dp->d_type = d_type; /* put infront of d_name*/
    /*  *((unsigned char*)ldp + ldp->d_reclen) = '\0'; */
    return dp;
}

/**
* The  closedir() function closes the directory stream associated with dirp.  A successful
* call to closedir() also closes the underlying file descriptor associated with dirp.  The
* directory stream descriptor dirp is not available after this call.
*/
int closedir(void *dirp) {
    int fd;
    struct dirstream *dir = dirp;
    if(!dirp || !dir->buf) {
        errno = EINVAL;
        return -1;
    }
    fd = dir->fd;
    free(dir->buf);
    free(dir);
    return close(fd);
}
