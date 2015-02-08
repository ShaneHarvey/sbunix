#include <unistd.h>
#include <stdlib.h>
#include <syscall.h>

/**
* Based on glibc srbk.
* Returns:  previous program break on success,
*           (void*)-1 on error
*/
void *sbrk(intptr_t increment) {
    /**
    * Calls the real Linux syscall, which returns:
    *   new break on success
    *   current break on failure
    * Here it will fail.
    */
    void *oldbreak = (void*)syscall_1(SYS_brk, (uint64_t)0);
    if(0 == brk(oldbreak + increment))
        return oldbreak;
    return (void*)-1;
}