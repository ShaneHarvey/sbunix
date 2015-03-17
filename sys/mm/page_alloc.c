#include <sbunix/sbunix.h>
#include <sbunix/mm/page_alloc.h>

struct freepagehd freepagehd = { .nfree = 0, .freepages = NULL};