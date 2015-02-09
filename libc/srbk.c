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
    intptr_t oldbreak = (intptr_t) syscall_1(SYS_brk, (uint64_t)0);
    if(0 == brk((void*)(oldbreak + increment)))
        return (void*)oldbreak;
    return (void*)-1;
}