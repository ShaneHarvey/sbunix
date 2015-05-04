#ifndef SBUNIX_MM_TYPES_H
#define SBUNIX_MM_TYPES_H

#include <sys/types.h>

/* Maybe don't use this and just add vm_ops inside vma struct */
typedef enum {
    VM_CODE,
    VM_RODATA,
    VM_DATA,
    VM_HEAP,
    VM_MMAP,
    VM_MMAP_ANON,
    VM_MMAP_FILE,
    VM_STACK
} vm_type_t;

/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */
struct vm_area {
    vm_type_t             vm_type;  /* VM_STACK, etc... */
    struct mm_struct      *vm_mm;   /* associated mm_struct */
    uint64_t              vm_start; /* VMA start, inclusive */
    uint64_t              vm_end;   /* VMA end , exclusive */
    struct vm_area        *vm_next; /* list of VMA's */
    uint64_t               vm_prot;  /* page tbl entry flags, pt.h */
    /* called by the page fault handler */
    int                  (*onfault) (struct vm_area *, uint64_t);
    struct file           *vm_file; /* mapped file, if any */
    off_t                 vm_fstart;/* starting offset into the file */
    size_t                vm_fsize; /* size of the file */
};

/* Fixme: work in progress */
/*
 * Describes the virtual memory information for a process.
 */
struct mm_struct {
    struct vm_area   *vmas;        /* list of memory areas */
    uint64_t         pml4;         /* page global directory */
    int              mm_count;     /* primary usage counter */
    int              vma_count;    /* number of memory areas */
    struct mm_struct *mm_prev;     /* list of all mm_structs */
    struct mm_struct *mm_next;     /* list of all mm_structs */
    uint64_t         start_code;   /* start address of code */
    uint64_t         end_code;     /* final address of code */
    uint64_t         start_rodata; /* start address of read only data */
    uint64_t         end_rodata;   /* final address of read only data */
    uint64_t         start_data;   /* start address of data */
    uint64_t         end_data;     /* final address of data */
    uint64_t         start_brk;    /* start address of heap */
    uint64_t         brk;          /* final address of heap */
    uint64_t         start_stack;  /* start address of stack */
    uint64_t         user_rsp;     /* entry user stack pointer */
    uint64_t         user_rip;     /* entry user instruction pointer */
    uint64_t         arg_start;    /* start of arguments */
    uint64_t         arg_end;      /* end of arguments */
    uint64_t         env_start;    /* start of environment */
    uint64_t         env_end;      /* end of environment */
    uint64_t         rss;          /* pages allocated */
    uint64_t         total_vm;     /* total number of pages */
};


#endif
