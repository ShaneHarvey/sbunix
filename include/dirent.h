#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/types.h>

/* Directories */

//typedef struct dirstream DIR;
struct dirstream {
    int fd;         /* Directory descriptor */
    size_t size;    /* Valid portion of buf, return of getdents(2) */
    size_t offset;  /* Current offset into buf */
    char buf[2048]; /* buffer of size space (holds  dirent structs) */
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


/* Actually used */
#define LDIR_NAMELEN(dp) ((dp)->d_reclen - 2 - (size_t)(((struct linux_dirent*)0)->d_name) )

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

void *opendir(const char *name);
struct dirent *readdir(void *dir);
int closedir(void *dirp);
int getdents(unsigned int fd, struct dirent *dirp, unsigned int count);

#endif
