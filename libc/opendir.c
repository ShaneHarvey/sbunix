#include <stdlib.h>

/**
* The opendir() function opens a directory stream corresponding to the directory name, and
* returns a pointer to the directory stream.  The stream is positioned at the first  entry
* in the directory.
*/
void *opendir(const char *name) {
    return NULL;
}

/**
* The readdir() function returns a pointer to a dirent  structure  representing  the  next
* directory entry in the directory stream pointed to by dirp.  It returns NULL on reaching
* the end of the directory stream or if an error occurred.
*/
struct dirent *readdir(void *dir) {
    return NULL;
}

/**
* The  closedir() function closes the directory stream associated with dirp.  A successful
* call to closedir() also closes the underlying file descriptor associated with dirp.  The
* directory stream descriptor dirp is not available after this call.
*/
int closedir(void *dir) {
    return 0;
}
