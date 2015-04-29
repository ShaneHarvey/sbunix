#include <sbunix/tarfs.h>

/* File hooks implemented by Tarfs */
struct file_ops tarfs_file_ops = {
    .lseek = tarfs_lseek,
    .read = tarfs_read,
    .write = tarfs_write,
//    .readdir = tarfs_readdir,
    .mmap = tarfs_mmap,
    .open = tarfs_open,
    .get_unmapped_area = tarfs_get_unmapped_area,
    .check_flags = tarfs_check_flags
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
 * Update the file pointer to the given offset.
 *
 * Upon successful completion, lseek() returns the resulting offset location
 * as  measured  in bytes from the beginning of the file.
 *
 * ERRORS Returns
 *  EBADF  fd is not an open file descriptor.
 *
 *  EINVAL whence  is  not  valid.   Or: the resulting file offset would be
 *         negative, or beyond the end of a seekable device.
 *
 *  EOVERFLOW The resulting file offset cannot be represented in an off_t.
 *
 *  ESPIPE fd is associated with a pipe, socket, or FIFO.
 */
off_t tarfs_lseek(struct file *fp, off_t offset, int origin) {
    return 0;
}

/**
 * Reads count bytes from the given file at position offset into buf. The file
 * pointer is then updated.
 */
ssize_t tarfs_read(struct file *fp, char *buf, size_t count, off_t *offset) {
    return 0;
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
 * Memory maps the given file onto the given address space
 */
int tarfs_mmap(struct file *fp, struct vm_area *vma) {
    return 0;
}

/**
 * Creates a new file object for the given path
 */
int tarfs_open(const char *path, struct file *fp) {

    return 0;
}

/**
 * Return an unused address space to map the given file
 */
unsigned long tarfs_get_unmapped_area(struct file *fp, unsigned long addr,
                                unsigned long len, unsigned long offset,
                                unsigned long flags) {
    return 0;
}

int tarfs_check_flags(int flags) {
    return 0;
}