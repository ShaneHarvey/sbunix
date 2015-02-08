#include <unistd.h>

/**
* Based on glibc srbk.
* Returns:  previous program break on success,
*           (void*)-1 on error
*/
void *sbrk(intptr_t increment) {
    void *oldbrk = brk(0xFFFF);
    return (void*)-1;
}