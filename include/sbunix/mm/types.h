#ifndef SBUNIX_MM_TYPES_H
#define SBUNIX_MM_TYPES_H

/* An address of a top level page table */
typedef uint64_t pgd_t;

/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */
struct vm_area_struct {
    struct mm_struct             *vm_mm;        /* associated mm_struct */
    unsigned long                vm_start;      /* VMA start, inclusive */
    unsigned long                vm_end;        /* VMA end , exclusive */
    struct vm_area_struct        *vm_next;      /* list of VMA's */
    /*pgprot_t                     vm_page_prot;*/  /* access permissions */
    unsigned long                vm_flags;      /* flags */
    struct vm_operations_struct  *vm_ops;           /* associated ops */
    unsigned long                vm_pgoff;          /* offset within file */
    struct file                  *vm_file;          /* mapped file, if any */
    void                         *vm_private_data;  /* private data */
};

/* Fixme: work in progress */
struct mm_struct {
    struct vm_area_struct  *mmap;               /* list of memory areas */
    pgd_t                  *pgd;                /* page global directory */
    /*atomic_t               mm_users;*/            /* address space users */
    int                    mm_count;            /* primary usage counter */
    int                    vma_count;           /* number of memory areas */
    /* struct rw_semaphore    mmap_sem; */            /* memory area semaphore */
    /* spinlock_t             page_table_lock; */     /* page table lock */
    struct mm_struct       *mm_prev;             /* list of all mm_structs */
    struct mm_struct       *mm_next;              /* list of all mm_structs */
    unsigned long          start_code;          /* start address of code */
    unsigned long          end_code;            /* final address of code */
    unsigned long          start_data;          /* start address of data */
    unsigned long          end_data;            /* final address of data */
    unsigned long          start_brk;           /* start address of heap */
    unsigned long          brk;                 /* final address of heap */
    unsigned long          start_stack;         /* start address of stack */
    unsigned long          arg_start;           /* start of arguments */
    unsigned long          arg_end;             /* end of arguments */
    unsigned long          env_start;           /* start of environment */
    unsigned long          env_end;             /* end of environment */
    unsigned long          rss;                 /* pages allocated */
    unsigned long          total_vm;            /* total number of pages */
};


#endif
