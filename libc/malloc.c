#include <stdlib.h>
#include <unistd.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define INC_PTR(ptr, inc) ((void*)(((char*)(ptr)) + (inc)))

/* TODO non-hardcoded PAGE_SIZE */
#define PAGE_SIZE 4096

/* extern struct freeblock *freelist;*/
static struct freeblock *freelist;

struct freeblock {
    size_t blocklen;  /* Always >= sizeof(struct freeblock) */
    struct freeblock *next;
    struct freeblock *prev;
};

/* Must be at least sizeof(struct freeblock) */
struct allocblock {
    size_t blocklen;  /* Always >= sizeof(struct freeblock) */
    /* ...  */
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
void swap_freeblock(struct freeblock *oldblock, struct freeblock *newblock) {
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
        swap_freeblock(oldblock, newblock);
    }
    else {
        /* we can't split, so rm oldblock */
        rm_freeblock(oldblock);
    }

}

/**
* TODO: insert inorder, not only at end
*/
void append_freelist(struct freeblock *newblock) {
    struct freeblock *curr = freelist;

    if(freelist == NULL) {
        freelist = newblock;
        return;
    }
    for(;curr->next != NULL; curr = curr->next) {

    }
    curr->next = newblock;
    newblock->prev = curr;
    return;
}

/**
* alloctest, using brk/sbrk
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
    size_t reqsize = MAX(sizeof(struct freeblock), size + 8);
    void *retptr;

    if(size == 0) {
        return NULL;
    }

    target = find_freeblock(reqsize);
    if(target != NULL) {
        if(target->blocklen == reqsize) {
            /* User gets a whole block */
            rm_freeblock(target);
            return INC_PTR(target, sizeof(size_t));
        }
        else if(target->blocklen > reqsize) {
            /* Gotta split a block */
            split_freeblock(target, reqsize);
            return INC_PTR(target, sizeof(size_t));
        }
        else {
            return NULL;
        }
    } else {
        /* Gotta get more mem */

        /* todo: don't assume reqsize <= PAGE_SIZE */
        target = sbrk((intptr_t)PAGE_SIZE);
        if(target == (struct freeblock*)-1) {
            /*errno = -57;  todo ENOMEM*/
            return NULL;
        }
        target->blocklen = reqsize;
        retptr = INC_PTR(target, sizeof(size_t)); /* ptr to return */
        target = INC_PTR(target, reqsize); /* ptr to remaining block*/
        target->blocklen = PAGE_SIZE - reqsize;
        target->next = target->prev = NULL;
        append_freelist(target);
        return retptr;
    }
    return NULL;
}


/**
* free
*/
void free(void *ptr) {

}
