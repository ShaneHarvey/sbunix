#include <sbunix/fs/tarfs.h>
#include <sbunix/string.h>
#include <sbunix/sbunix.h>
#include <errno.h>


#define MIN(a, b) (((a)<(b))?(a):(b))

/* File hooks implemented by Tarfs */
struct file_ops tarfs_file_ops = {
    .lseek = tarfs_lseek,
    .read = tarfs_read,
    .write = tarfs_write,
//    .readdir = tarfs_readdir,
    .close = tarfs_close
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
    fp = tarfs_open(path, 0, 0, &err);
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
 * TODO: flags
 * @path: The absolute path of the file to open (MUST start with '/')
 * @flags: user open flags
 * @mode: unused
 * @err: pointer to error indicator
 */
struct file *tarfs_open(const char *path, int flags, mode_t mode, int *err) {
    struct posix_header_ustar *hd;
    struct file *fp;

    if(!path || *path != '/')
        kpanic("path must start with / !!!\n");

    for(hd = tarfs_first(); hd != NULL; hd = tarfs_next(hd)) {
        if(strncmp(path+1, hd->name, sizeof(hd->name)) == 0) {
            fp = kmalloc(sizeof(struct file));
            if(!fp) {
                *err = -ENOMEM;
                return NULL;
            }
            fp->private_data = hd;
            fp->f_size = aotoi(hd->size, sizeof(hd->size));
            fp->f_pos = 0;
            fp->f_error = 0;
            fp->f_op = &tarfs_file_ops;
            fp->f_flags = flags;
            fp->f_count = 1;
            *err = 0;
            return fp;
        }
    }
    *err = -ENOENT;
    return NULL;
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
    /* Error checking */
    if(!fp)
        return -EBADF;
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
    if(!tarfs_isfile(hd) || *offset >= fp->f_size)
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
 * Return the next directory in a directory listing.
 */
//int tarfs_readdir(struct file *fp, void *dirent, filldir_t filldir) {
//    return 0;
//}

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
 * Return 0 if the absolute path is a directory
 */
long tarfs_isdir(const char *abspath) {
    //return -ENOENT;
    return -ENOTDIR;
}
