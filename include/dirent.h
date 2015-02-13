#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/types.h>

/* Directories */

typedef struct dirstream DIR;
struct dirstream {
    int fd;         /* Directory descriptor */
    size_t space;   /* Size of buf */
    size_t size;    /* Valid portion of buf, return of getdents(2) */
    size_t offset;  /* Current offset into buf */
    off_t filepos;  /* Offset of next entry to read, linux_dirent.d_off */
    char *buf;      /* buffer of size space */
};

#define NAME_MAX 255

struct dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name [NAME_MAX+1];
};

#define DT_UNKNOWN      0
#define DT_FIFO         1
#define DT_CHR          2
#define DT_DIR          4
#define DT_BLK          6
#define DT_REG          8
#define DT_LNK          10
#define DT_SOCK         12
#define DT_WHT          14

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
int getdents(unsigned int fd, struct dirent *dirp, unsigned int count);

#endif
