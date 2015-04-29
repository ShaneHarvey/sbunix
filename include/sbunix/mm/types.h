#ifndef SBUNIX_MM_TYPES_H
#define SBUNIX_MM_TYPES_H

/* An address of a top level page table */
typedef uint64_t pgd_t;

/* Maybe don't use this and just add vm_ops inside vma struct */
typedef enum {
    VM_TEXT,
    VM_RODATA,
    VM_RWDATA,
    VM_HEAP,
    VM_MMAP_ANON,
    VM_MMAP_FILE,
    VM_STACK
} vm_type_t;

/* Bit flags for vm_prot */
#define VM_READ     1
#define VM_WRITE    2
#define VM_EXEC     4

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
    ulong                 vm_prot;  /* access permissions */
    /* called by the page fault handler */
    int                   (*onfault) (struct vm_area *, uint64_t);
    /*unsigned long         vm_pgoff;*/ /* offset within file */
    struct file           *vm_file; /* mapped file, if any */
};

/* Fixme: work in progress */
struct mm_struct {
    struct vm_area   *vmas;        /* list of memory areas */
    pgd_t            *pgd;         /* page global directory */
    int              mm_count;     /* primary usage counter */
    int              vma_count;    /* number of memory areas */
    struct mm_struct *mm_prev;     /* list of all mm_structs */
    struct mm_struct *mm_next;     /* list of all mm_structs */
    uint64_t         start_code;   /* start address of code */
    uint64_t         end_code;     /* final address of code */
    uint64_t         start_data;   /* start address of data */
    uint64_t         end_data;     /* final address of data */
    uint64_t         start_brk;    /* start address of heap */
    uint64_t         brk;          /* final address of heap */
    uint64_t         start_stack;  /* start address of stack */
    uint64_t         arg_start;    /* start of arguments */
    uint64_t         arg_end;      /* end of arguments */
    uint64_t         env_start;    /* start of environment */
    uint64_t         env_end;      /* end of environment */
    uint64_t         rss;          /* pages allocated */
    uint64_t         total_vm;     /* total number of pages */
};


#endif
