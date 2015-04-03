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
    *   new break     -- on success
    *   current break -- on failure
    * Here it will fail and therefore return the current break (oldbrk)
    */
    intptr_t oldbrk = (intptr_t) syscall_1(SYS_brk, (uint64_t)0);
    if(increment == 0)
        return (void*) oldbrk;

    if(0 == brk((void*)(oldbrk + increment)))
        return (void*) oldbrk;
    return (void*)-1;
}