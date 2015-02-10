#ifndef _MMAN_H
#define _MMAN_H

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
        off_t offset);

int munmap(void *addr, size_t length);

#endif
