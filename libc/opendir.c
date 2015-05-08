#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>


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
    if(fd < 0)
        return NULL;

    dirp = malloc(sizeof(*dirp));
    if(!dirp)
        return NULL;
    dirp->fd = fd;
    dirp->offset = dirp->size = 0;
    return dirp;
}

/**
* The readdir() function returns a pointer to a dirent  structure  representing  the  next
* directory entry in the directory stream pointed to by dirp.  It returns NULL on reaching
* the end of the directory stream or if an error occurred.
*/
struct dirent *readdir(void *dirp) {
    struct dirent *dp = NULL;
    struct dirstream *dir = dirp;
    int bytesread = 0;
    printf("BEFORE readdir\n");
    printf("dir->fd = %d\n",  dir->fd);
    printf("dir->size = %d\n", (int)dir->size);
    printf("dir->offset = %d\n", (int)dir->offset);

    if(dir->offset >= dir->size) {
        /* Must call getdents */
        errno = 0;
        bytesread = getdents((uint) dir->fd, (struct dirent*) dir->buf, (uint) sizeof(dir->buf));
        if(bytesread <= 0)
            return NULL; /* End of dir, or error (errno set) */

        dir->size = (size_t)bytesread;
        dir->offset = 0;
    }
    dp = (struct dirent*)(dir->buf + dir->offset);
    dir->offset += dp->d_reclen;
    printf("AFTER readdir\n");
    printf("dir->fd = %d\n",  dir->fd);
    printf("dir->size = %d\n", (int)dir->size);
    printf("dir->offset = %d\n", (int)dir->offset);
//
//    /* convert linux_dirent to dirent */
//    d_type = *((unsigned char*)ldp + ldp->d_reclen - 1); /* save */
//    memmove(dp->d_name, ldp->d_name, LDIR_NAMELEN(ldp) + 1); /* shift down 1 */
//    dp->d_type = d_type; /* put infront of d_name*/
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
    free(dir);
    return close(fd);
}
