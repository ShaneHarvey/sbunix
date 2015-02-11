#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <debug.h>

#define MAX(a, b)           (((a)>(b))?(a):(b))
#define INC_PTR(ptr, inc)   ((void*)(((char*)(ptr)) + (inc)))

/*#define _DEBUG_MALLOC*/

/* TODO non-hardcoded PAGE_SIZE */
#define PAGE_SIZE 4096

/* todo: extern struct freeblock *freelist;*/
static struct freeblock *freelist = NULL;

struct freeblock {
    size_t blocklen;  /* Always >= sizeof(struct freeblock) */
    struct freeblock *next;
    struct freeblock *prev;
};


/**
* Finds a freeblock.
* @reqsize  MAX of user's size and sizeof(struct freeblock)
* Return: pointer to freeblock to use
*       NULL if no suitable block found
*/
struct freeblock *find_freeblock(size_t reqsize) {
    struct freeblock *curr = freelist;
    struct freeblock *target = NULL;
    for(;curr != NULL; curr = curr->next) {
        if(curr->blocklen == reqsize) {
            /* An exact match is best */
            target = curr;
            break;
        } else if(curr->blocklen > reqsize) {
            /* If no exact match we want the next smallest */
            if(target == NULL || curr->blocklen < target->blocklen) {
                target = curr;
            }
        }
    }

    return target;
}

/**
* Removes this freeblock from the freelist doubly-linked list.
*/
void rm_freeblock(struct freeblock *rmblock) {
    if(rmblock == freelist) {
        /* Replace the head freeblock */
        freelist = rmblock->next;
        if(freelist != NULL) {
            freelist->prev = NULL;
        }
    } else {
        /* If not head, then it must had a prev */
        rmblock->prev->next = rmblock->next;

        if (rmblock->next != NULL) {
            rmblock->next->prev = rmblock->prev;
        }
    }
}

/**
* Replace oldblock with newblock in the doubly-linked freelist.
*/
void replace_freeblock(struct freeblock *oldblock, struct freeblock *newblock) {
    newblock->prev = NULL;
    newblock->next = NULL;
    if(oldblock == freelist) {
        freelist = newblock;
    }
    if(oldblock->prev != NULL) {
        newblock->prev = oldblock->prev;
        oldblock->prev->next = newblock;
    }
    if(oldblock->next != NULL) {
        newblock->next = oldblock->next;
        oldblock->next->prev = newblock;
    }
}

/**
* Splits off the first section of a freeblock, if needed.
* @reqsize  MAX of user's size and sizeof(struct freeblock)
*/
void split_freeblock(struct freeblock *oldblock, size_t reqsize) {
    struct freeblock *newblock;
    size_t newlen = oldblock->blocklen - reqsize;
    if(newlen >= sizeof(struct freeblock)) {
        /* we can split it */
        newblock = INC_PTR(oldblock, reqsize);
        oldblock->blocklen = reqsize;
        newblock->blocklen = newlen;
        replace_freeblock(oldblock, newblock);
    }
    else {
        /* we can't split, so rm oldblock */
        rm_freeblock(oldblock);
    }

}

void coalesce_freelist(struct freeblock *start) {
    struct freeblock *curr = start;
    if(!curr) {
        return;
    }

    while(curr != NULL) {
        if(INC_PTR(curr, curr->blocklen) == curr->next) {
            /* New can before-merge with curr to curr->next */
            curr->blocklen += curr->next->blocklen;
            if(curr->next->next) {
                curr->next->next->prev = curr;
            }
            curr->next = curr->next->next;
            continue;
        }
        curr = curr->next;
    }
}

/**
* Insert into the list in sorted order.
* Also coalesces the blocks if possible.
*/
void append_freelist(struct freeblock *newblock) {
    struct freeblock *curr = freelist;
    struct freeblock *prev = NULL;
    if(curr == NULL) {
        /* Head is NULL */
        freelist = newblock;
        return;
    }
    for(;curr != NULL; prev = curr, curr = curr->next) {
        if(newblock < curr) {
            /* Insert into list */
            newblock->next = curr;
            newblock->prev = prev;
            curr->prev = newblock;
            if(prev) {
                prev->next = newblock;
            }
            if(curr == freelist) {
                freelist = newblock; /* new head */
            }
            break;
        }

    }
    if(curr == NULL) {
        /* prev is at the end of the list */
        prev->next = newblock;
        newblock->prev = prev;
        newblock->next = NULL;
    }
    coalesce_freelist(freelist);
    return;
}

/* Print blocks */
void printfreelist(void) {
#ifdef _DEBUG_MALLOC
    struct freeblock *curr = freelist;
    printf(_RED"Printing freelist:\n"_RESET);
    for(; curr != NULL; curr = curr->next) {
        printf("       ------ %d ------\n", curr);
        printf("       | size: %d\n", curr->blocklen);
        printf("       | next: %d\n", curr->next);
        printf("       | prev: %d\n", curr->prev);
        printf("       ---------------------\n");
    }
#endif
}

/**
* malloc, using brk/sbrk
* Loop through free list to find larger enough block:
*   * should stop when finding an exact match
*   * o.w. split the smallest suitable freeblock into 2 blocks
*       * (24+24) <= block->blocklen >= size
*
* Else call brk/sbrk
*
* Returns: NULL if size is 0 or NULL on error
*/
void *malloc(size_t size) {
    struct freeblock *target = NULL;
    size_t reqsize = MAX(sizeof(struct freeblock), size + sizeof(size_t));
    void *retptr = NULL;

    if(size == 0) {
        return NULL;
    }

    target = find_freeblock(reqsize);
    if(target != NULL) {
        if(target->blocklen == reqsize) {
            /* User gets a whole block */
            rm_freeblock(target);
            retptr = INC_PTR(target, sizeof(size_t));
        }
        else if(target->blocklen > reqsize) {
            /* Gotta split a block */
            split_freeblock(target, reqsize);
            retptr = INC_PTR(target, sizeof(size_t));
        }
    } else {
        /* Gotta get more mem */
        intptr_t increment = ((reqsize/PAGE_SIZE) + 1) * PAGE_SIZE;
        target = sbrk(increment);
        if(target == (struct freeblock*)-1) {
            errno = ENOMEM;
            return NULL;
        } else {
            target->blocklen = reqsize;
            retptr = INC_PTR(target, sizeof(size_t)); /* ptr to return */
            target = INC_PTR(target, reqsize); /* ptr to remaining block*/
            target->blocklen = increment - reqsize;
            target->next = target->prev = NULL;
            append_freelist(target);
        }
    }
    printfreelist();
    return retptr;
}


/**
* free, opposite of malloc
*/
void free(void *ptr) {
    struct freeblock *block = INC_PTR(ptr, (-1 * sizeof(size_t)));
    if(!ptr) {
        /* Allow free(NULL) */
        return;
    }
    block->next = NULL;
    block->prev = NULL;
    /* zero fill free'd space */
    memset(INC_PTR(block, sizeof(size_t)), 0, block->blocklen - sizeof(size_t));
    append_freelist(block);
    printfreelist();
}

/**
* realloc, standard realloc.
* See if we can extend
*/
void *realloc(void *ptr, size_t size) {
    size_t oldsize, reqsize;
    if(!ptr) {
        return malloc(size);
    }
    reqsize = MAX(sizeof(struct freeblock), size + sizeof(size_t));
    oldsize = *(size_t*)INC_PTR(ptr, (-1 * sizeof(size_t)));

    if(reqsize == oldsize) {
        return ptr;
    } else if(reqsize < oldsize) {
        if(oldsize - reqsize >= sizeof(struct freeblock)) {
            struct freeblock *newblock = INC_PTR(ptr, reqsize - sizeof(size_t));
            newblock->blocklen = oldsize - reqsize;
            append_freelist(newblock);
            *(size_t*)INC_PTR(ptr, (-1 * sizeof(size_t))) = reqsize;
        }
        printfreelist();
        return ptr;
    } else {
        /* todo: could improve to only malloc() if needed */
        void *newptr = malloc(size);
        if(!newptr) {
            free(ptr);
            return NULL;
        }
        memcpy(newptr, ptr, size);
        free(ptr);
        return newptr;
    }
}