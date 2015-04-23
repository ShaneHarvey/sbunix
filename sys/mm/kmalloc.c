#include <sbunix/sbunix.h>
#include <sbunix/mm/kmalloc.h>

/**
 * For allocations smaller than one PAGE_SIZE.
 * Must be kfree()'d.
 * @return: kernel virtual pointer to memory of length size
 * TODO: Don't just call get_free_page()
 */
void* kmalloc(size_t size) {
    if(size == 0)
        return 0;

    if(size > PAGE_SIZE) {
        printk("kmalloc(0x%lu) too big! Use get_free_page", size);
        return NULL;
    }
    return (void*)get_free_page(0);
}

/**
 * Free a pointer returned by kmalloc().
 */
void kfree(void* ptr) {
    if(!ptr)
        return;
    free_page((uint64_t)ptr);
}