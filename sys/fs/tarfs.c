#include <sbunix/fs/tarfs.h>
#include <sbunix/string.h>
#include <sbunix/sbunix.h>
#include <dirent.h>
#include <errno.h>


#define MIN(a, b) (((a)<(b))?(a):(b))

/* File hooks implemented by Tarfs */
struct file_ops tarfs_file_ops = {
    .lseek = tarfs_lseek,
    .read = tarfs_read,
    .write = tarfs_write,
    .readdir = tarfs_readdir,
    .close = tarfs_close,
    .can_mmap = tarfs_can_mmap
};

/* open "/" for use with readdir */
struct posix_header_ustar fs_root_hdr = {
        .name = "", /* or "/"? */
        .mode = {0},
        .uid = {0},
        .gid = {0},
        .size = {0},
        .mtime = {0},
        .checksum = {0},
        .typeflag = TARFS_DIRECTORY,
        .linkname = {0},
        .magic = "ustar",
        .version = {0},
        .uname = {0},
        .gname = {0},
        .devmajor = {0},
        .devminor = {0},
        .prefix = {0},
        .pad = {0}
};

/**
 * Ascii Octal To 64-bit unsigned Integer
 * @optr: octal string to parse
 * @length: length of the string to parse
 */
uint64_t aotoi(char *optr, int length) {
    uint64_t val = 0;
    if(optr == NULL)
        return 0;
    while(length-- > 0 && *optr >= '0' && *optr <= '7') {
        val <<= 3; /* multiply by 8 (the base) */
        val += *optr++ - '0';
    }
    return val;
}

/**
 * Short test of aotoi
 */
void test_aotoi(void) {
    char *octals[] = {"01234567", "055", "01", "04672", ""};
    uint64_t ints[] = {342391ULL, 45ULL, 1ULL, 2490ULL, 0ULL};
    int i;

    for(i = 0; i < 5; i++) {
        if (ints[i] != aotoi(octals[i], 12)) {
            kpanic("octal %s != decimal %lu!!\n", octals[i], aotoi(octals[i], 12));
        }
    }
}

void tarfs_print(struct posix_header_ustar *hd) {
    const char *type;
    uint64_t size;
    if(hd == NULL)
        return;

    switch(hd->typeflag) {
        case 0:
        case '0': type = "normal file";break;
        case '5': type = "directory";break;
        default:  type = "unsupported";break;
    }
    size = aotoi(hd->size, sizeof(hd->size));
    printk("%s%s: %s, %lu bytes\n", hd->prefix, hd->name, type, size);
}

/**
 * Test tarfs by printing all files in the archive
 */
void test_read_tarfs(void) {
    struct posix_header_ustar *hd;

    printk("Test reading tarfs:\n");
    for(hd = tarfs_first(); hd != NULL; hd = tarfs_next(hd)) {
        tarfs_print(hd);
    }
    printk("Done reading tarfs\n");
}

/**
 * Test open/read
 */
void test_all_tarfs(const char *path) {
    struct file *fp;
    int err;
    ssize_t bytes;
    char buf[10];

    printk("Tarfs test: open/read/close on %s\n", path);
    fp = tarfs_open(path, O_RDONLY, 0, &err);
    if(err) {
        printk("Error open: %s\n", strerror(-err));
        return;
    }
    bytes = fp->f_op->read(fp, buf, sizeof(buf) - 1, &fp->f_pos);
    if(bytes < 0) {
        printk("Error read: %s\n", strerror((int)-bytes));
        return;
    }
    buf[sizeof(buf) - 1] = 0;
    printk("Read %d bytes, data: '%s'\n", (int)bytes, buf);

    err = fp->f_op->close(fp);
    if(err) {
        printk("Error close: %s\n", strerror(-err));
        return;
    }
    printk("Finished open/read/close\n");
}

/**
 * Creates a new file object for the given path
 *
 * @path: The absolute path of the file to open (MUST start with '/')
 * @flags: user open flags
 * @mode: unused
 * @err: pointer to error indicator
 */
struct file *tarfs_open(const char *path, int flags, mode_t mode, int *err) {
    struct posix_header_ustar *hd;
    struct file *fp;
    int is_root = 0;

    if(!path || *path != '/')
        kpanic("path invalid '%s'!!!\n", path);

    if(flags & (O_WRONLY | O_RDWR | O_CREAT)) {
        *err = -EACCES;
        return NULL;
    }
    if(path[1] == '\0') { /* path is "/" */
        is_root = 1;
        hd = &fs_root_hdr;
        goto found_it;
    }

    for(hd = tarfs_first(); hd != NULL; hd = tarfs_next(hd)) {
        if(strncmp(path+1, hd->name, sizeof(hd->name)) == 0) {
            if(flags & O_DIRECTORY && hd->typeflag != TARFS_DIRECTORY) {
                *err = -ENOTDIR;
                return NULL;
            }
            goto found_it;
        }
    }
    *err = -ENOENT;
    return NULL;
found_it:
    fp = kmalloc(sizeof(struct file));
    if(!fp) {
        *err = -ENOMEM;
        return NULL;
    }
    fp->private_data = hd;
    fp->f_size = aotoi(hd->size, sizeof(hd->size));
    /* save index of next header (used by readdir) */
    if(is_root)
        fp->f_pos = (uint64_t)tarfs_first();  /* root points to first header */
    else if(hd->typeflag == TARFS_DIRECTORY)
        fp->f_pos = (uint64_t)tarfs_next(hd); /* other dirs point to next header */
    else
        fp->f_pos = 0;
    fp->f_error = 0;
    fp->f_op = &tarfs_file_ops;
    fp->f_flags = flags;
    fp->f_count = 1;
    *err = 0;
    return fp;
}

/**
 * Update the file pointer to the given offset.
 *
 * Upon successful completion, lseek() returns the resulting offset location
 * as measured in bytes from the beginning of the file.
 *
 * @whence: SEEK_SET, offset is set to offset bytes.
 *          SEEK_CUR, offset is set to its current location plus offset bytes.
 *          SEEK_END, offset is set to the size of the file plus offset bytes.
 *
 * ERRORS Returns
 *  EBADF  fd is not an open file descriptor.
 *
 *  EINVAL whence is not valid. Or: the resulting file offset would be
 *         negative, or beyond the end of a seekable device.
 *
 *  EOVERFLOW The resulting file offset cannot be represented in an off_t.
 */
off_t tarfs_lseek(struct file *fp, off_t offset, int whence) {
    off_t new_off;
    struct posix_header_ustar *hd;
    /* Error checking */
    if(!fp)
        return -EBADF;
    hd = (struct posix_header_ustar *)fp->private_data;
    if(hd->typeflag == TARFS_DIRECTORY)
        return -EISDIR;

    if(!tarfs_normal_type(hd))
        return -EINVAL;

    switch(whence) {
        case SEEK_SET:
            if(offset < 0 || offset >= fp->f_size)
                return -EINVAL;
            fp->f_pos = offset;
            break;
        case SEEK_CUR:
            /* TODO: overflow detection */
            new_off = fp->f_pos + offset;
            if(new_off < 0 || new_off >= fp->f_size)
                return -EINVAL;
            fp->f_pos = new_off;
            break;
        case SEEK_END:
            /* TODO: is this f_size or F_size - 1??? */
            new_off = (off_t)fp->f_size + offset;
            if(new_off < 0 || new_off >= fp->f_size)
                return -EINVAL;
            fp->f_pos = new_off;
            break;
        default:
            return -EINVAL;
    }
    return fp->f_pos;
}

/**
 * Reads count bytes from the given file at position offset into buf. The file
 * pointer is then updated.
 *
 * @fp:  pointer to a file struct backed by a tarfs file
 * @buf: user buffer to read into
 * @count: maximum number of bytes to read
 * @offset: offset from the beginning of the file to start reading
 */
ssize_t tarfs_read(struct file *fp, char *buf, size_t count, off_t *offset) {
    struct posix_header_ustar *hd;
    char *file_data_start;
    size_t bytes_left, num_read;

    /* Error checking */
    if(!fp || !buf || !offset || *offset < 0)
        return -EINVAL;
    hd = (struct posix_header_ustar *)fp->private_data;
    if(hd->typeflag == TARFS_DIRECTORY)
        return -EISDIR;

    if(!tarfs_normal_type(hd) || *offset >= fp->f_size)
        return -EINVAL;
    /* Do read */
    if(count == 0)
        return 0;
    bytes_left = fp->f_size - *offset;
    num_read = MIN(bytes_left, count);
    file_data_start = (char *)(hd + 1);
//    debug("bytes_left=%d, offset=%d, num_read=%d, count=%d\n",
//          (int)bytes_left, (int)*offset, (int)num_read, (int)count);
    memcpy(buf, file_data_start + *offset, num_read);
    *offset += num_read;
    return num_read;
}

/**
 * As tarfs is read-only this should be result in an error.
 */
ssize_t tarfs_write(struct file *fp, const char *buf, size_t count,
                    off_t *offset) {
    /* TODO: Do some error notification thing */
    return -EINVAL;
}

/**
 * Return the next file in a directory listing.
 * Read one dirent from filep into buf, if the size permits.
 *
 * @filep: validated in sys_getdents/do_getdents
 * @buf:   validated in sys_getdents/do_getdents
 * @count: size of buf
 */
int tarfs_readdir(struct file *filep, void *buf, unsigned int count) {
    struct dirent *dent;
    struct posix_header_ustar *next, *fhdr;
    unsigned int size;
    size_t len, elen;
    const char *entryname;
    unsigned char type;


    fhdr = filep->private_data;
    len = strlen(fhdr->name);
//    printk("READDIR: %s\n", fhdr->name);

    next = (struct posix_header_ustar *)filep->f_pos;
    for( ;next != NULL; next = tarfs_next(next)) {
//        printk("READDIR: next header: %s\n", next->name);
        /* check in next's name begins with this dir name */
        if(strncmp(fhdr->name, next->name, len) != 0)
            break; /* no more entries */

        /* filename on potential entry */
        entryname = next->name + len + 1;
        if(strchr(entryname, '/'))
            continue; /* skip to next header */

//        printk("READDIR: found entry: %s\n", entryname);
        elen = strlen(entryname); /* need to copy this name to buf dirent */
        dent = buf;
        size = (__builtin_offsetof(struct dirent, d_name) + elen + 1);
        if(count < size) {
            filep->f_pos = (uint64_t) next; /* save for next call */
            return -EINVAL;
        }
        dent->d_ino = (ulong)next; /* unique "inode" for this file */
        dent->d_off = 0;
        dent->d_reclen = (unsigned short) size;
        strcpy(dent->d_name, entryname);

        if(next->typeflag == TARFS_DIRECTORY)
            type = DT_DIR;
        else if(tarfs_normal_type(next))
            type = DT_REG;
        else
            type = DT_UNKNOWN;
        dent->d_type = type ;  /* File type byte */
        filep->f_pos = (uint64_t) tarfs_next(next); /* save for next call */
        return size;
    }
    /* no more entries */
    filep->f_pos = 0;
    return 0;
}

/**
 * Called while closing a file descriptor to free any information related
 * to the file.
 * Tarfs simply zeros out the memory and returns success
 */
int tarfs_close(struct file *fp) {
    if(!fp)
        kpanic("file is NULL!!!\n");
    fp->f_count--;
    if(fp->f_count == 0) {
        memset(fp, 0, sizeof(struct file));
        kfree(fp);
    }
    return 0;
}

/**
 * Can user mmap this file?
 */
int tarfs_can_mmap(struct file *fp) {
    if(!fp)
        kpanic("file is NULL!!!\n");

    if(!tarfs_normal_type(fp->private_data)) {
        return -EACCES;
    }

    return 0;
}

/**
 * Return 0 if the absolute path is a supported normal file
 */
long tarfs_isnormal(const char *abspath) {
    struct posix_header_ustar *hd;

    if(abspath[0] == '/' && abspath[1] == '\0')
        return -EISDIR;

    for(hd = tarfs_first(); hd != NULL; hd = tarfs_next(hd)) {
        if(strncmp(abspath+1, hd->name, sizeof(hd->name)) == 0) {
            if(hd->typeflag == TARFS_DIRECTORY)
                return -EISDIR;
            else if(tarfs_normal_type(hd))
                return 0;
            else
                /* file type not supported */
                return -EACCES;
        }
    }
    return -ENOENT;
}

/**
 * Return 0 if the absolute path is a supported file or directory
 */
long tarfs_isfile(const char *abspath) {
    struct posix_header_ustar *hd;

    if(abspath[0] == '/' && abspath[1] == '\0')
        return 0;

    for(hd = tarfs_first(); hd != NULL; hd = tarfs_next(hd)) {
        if(strncmp(abspath+1, hd->name, sizeof(hd->name)) == 0) {
            if(hd->typeflag == TARFS_DIRECTORY)
                return 0;
            else if(tarfs_normal_type(hd))
                return 0;
            else
                /* file type not supported */
                return -EACCES;
        }
    }
    return -ENOENT;
}

/**
 * Return 0 if the absolute path is a directory
 */
long tarfs_isdir(const char *abspath) {
    struct posix_header_ustar *hd;

    if(abspath[0] == '/' && abspath[1] == '\0')
        return 0;

    for(hd = tarfs_first(); hd != NULL; hd = tarfs_next(hd)) {
        if(strncmp(abspath+1, hd->name, sizeof(hd->name)) == 0) {
            if(hd->typeflag == TARFS_DIRECTORY)
                return 0;
            else
                return -ENOTDIR;
        }
    }
    return -ENOENT;
}

/**
 * removes all the trailing slashes from tar file names
 */
long tarfs_init(void) {
    struct posix_header_ustar *hd;
    size_t len;

    for(hd = tarfs_first(); hd != NULL; hd = tarfs_next(hd)) {
        if(hd->typeflag != TARFS_DIRECTORY)
            continue;
        /* remove trailing slash */
        len = strnlen(hd->name, sizeof(hd->name));
        if(hd->name[len-1] == '/')
            hd->name[len-1] = '\0';
    }
    return 0;
}
