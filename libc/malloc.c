#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define MAX(a, b)           (((a)>(b))?(a):(b))
#define MIN(a, b)           (((a)<(b))?(a):(b))
#define INC_PTR(ptr, inc)   ((void*)(((char*)(ptr)) + (inc)))

/*#define _DEBUG_MALLOC*/

/* TODO non-hardcoded PAGE_SIZE */
#define PAGE_SIZE 4096

/* todo: extern struct __freeblk *_freehd;*/
static struct _freeblk *_freehd = NULL;

struct _freeblk {
    size_t blocklen;  /* Always >= sizeof(struct _freeblk) */
    struct _freeblk *next;
    struct _freeblk *prev;
};


/**
* Finds a _freeblk.
* @reqsize  MAX of user's size and sizeof(struct _freeblk)
* Return: pointer to _freeblk to use
*       NULL if no suitable block found
*/
struct _freeblk *_find_freeblk(size_t reqsize) {
    struct _freeblk *curr = _freehd;
    struct _freeblk *target = NULL;
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
* Removes this _freeblk from the freelist doubly-linked list.
*/
void _rm_freeblk(struct _freeblk *rmblock) {
    if(rmblock == _freehd) {
        /* Replace the head _freeblk */
        _freehd = rmblock->next;
        if(_freehd != NULL) {
            _freehd->prev = NULL;
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
void _replace_freeblk(struct _freeblk *oldblock, struct _freeblk *newblock) {
    newblock->prev = NULL;
    newblock->next = NULL;
    if(oldblock == _freehd) {
        _freehd = newblock;
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
* Splits off the first section of a _freeblk, if needed.
* @reqsize  MAX of user's size and sizeof(struct _freeblk)
*/
void _split_freeblk(struct _freeblk *oldblock, size_t reqsize) {
    struct _freeblk *newblock;
    size_t newlen = oldblock->blocklen - reqsize;
    if(newlen >= sizeof(struct _freeblk)) {
        /* we can split it */
        newblock = INC_PTR(oldblock, reqsize);
        oldblock->blocklen = reqsize;
        newblock->blocklen = newlen;
        _replace_freeblk(oldblock, newblock);
    }
    else {
        /* we can't split, so rm oldblock */
        _rm_freeblk(oldblock);
    }

}

void _coalesce_freelist(struct _freeblk *start) {
    struct _freeblk *curr = start;
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
void _append_freelist(struct _freeblk *newblock) {
    struct _freeblk *curr = _freehd;
    struct _freeblk *prev = NULL;
    if(curr == NULL) {
        /* Head is NULL */
        _freehd = newblock;
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
            if(curr == _freehd) {
                _freehd = newblock; /* new head */
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
    _coalesce_freelist(_freehd);
    return;
}

/* Print blocks */
void _printfreelist(void) {
#ifdef _DEBUG_MALLOC
    struct _freeblk *curr = _freehd;
    printf("Printing freelist:\n");
    for(; curr != NULL; curr = curr->next) {
        printf("       ------ %p ------\n", curr);
        printf("       | size: %lu\n", curr->blocklen);
        printf("       | next: %p\n", curr->next);
        printf("       | prev: %p\n", curr->prev);
        printf("       ---------------------\n");
    }
#endif
}

/**
* malloc, using brk/sbrk
* Loop through free list to find larger enough block:
*   * should stop when finding an exact match
*   * o.w. split the smallest suitable _freeblk into 2 blocks
*       * (24+24) <= block->blocklen >= size
*
* Else call brk/sbrk
*
* Returns: NULL if size is 0 or NULL on error
*/
void *malloc(size_t size) {
    struct _freeblk *target = NULL;
    size_t reqsize = MAX(sizeof(struct _freeblk), size + sizeof(size_t));
    void *retptr = NULL;

    if(size == 0) {
        return NULL;
    }

    target = _find_freeblk(reqsize);
    if(target != NULL) {
        if(target->blocklen == reqsize) {
            /* User gets a whole block */
            _rm_freeblk(target);
            retptr = INC_PTR(target, sizeof(size_t));
        }
        else if(target->blocklen > reqsize) {
            /* Gotta split a block */
            _split_freeblk(target, reqsize);
            retptr = INC_PTR(target, sizeof(size_t));
        }
    } else {
        /* Gotta get more mem */
        intptr_t increment = ((reqsize/PAGE_SIZE) + 1) * PAGE_SIZE;
        target = sbrk(increment);
        if(target == (struct _freeblk*)-1) {
            /* errno = ENOMEM;  Set by sbrk() */
            return NULL;
        } else {
            target->blocklen = reqsize;
            retptr = INC_PTR(target, sizeof(size_t)); /* ptr to return */
            target = INC_PTR(target, reqsize); /* ptr to remaining block*/
            target->blocklen = increment - reqsize;
            target->next = target->prev = NULL;
            _append_freelist(target);
        }
    }
    _printfreelist();
    return retptr;
}


/**
* free, opposite of malloc
*/
void free(void *ptr) {
    struct _freeblk *block = INC_PTR(ptr, (-1 * sizeof(size_t)));
    if(!ptr) {
        /* Allow free(NULL) */
        return;
    }
    block->next = NULL;
    block->prev = NULL;
    /* zero fill free'd space */
    memset(INC_PTR(block, sizeof(size_t)), 0, block->blocklen - sizeof(size_t));
    _append_freelist(block);
    _printfreelist();
}

/**
* calloc, standard calloc
*/
void *calloc(size_t nmemb, size_t size) {
    void *rv = malloc(nmemb * size);
    if(rv) {
        memset(rv, 0, nmemb * size);
    }
    return rv;
}

/**
* realloc, standard realloc.
*/
void *realloc(void *ptr, size_t size) {
    size_t oldsize, reqsize;
    if(!ptr)
        return malloc(size);

    reqsize = MAX(sizeof(struct _freeblk), size + sizeof(size_t));
    oldsize = *(size_t*)INC_PTR(ptr, (-1 * sizeof(size_t)));

    if(reqsize == oldsize) {
        return ptr;
    } else {
        void *newptr = malloc(size); /* Should be size not reqsize */
        if(!newptr)
            return NULL;

        memcpy(newptr, ptr, MIN(oldsize, size));
        free(ptr);
        return newptr;
    }
}