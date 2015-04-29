#ifndef _SBUNIX_SCHED_H
#define _SBUNIX_SCHED_H

#include <sys/defs.h>
#include <sbunix/mm/types.h> /* mm_struct */

extern struct rq            run_queue;
extern struct mm_struct     kernel_mm;
extern struct task_struct   kernel_task;

/* Kernel thread or user process */
struct task_struct {
    int type;
    int state;
    int flags;
    uint64_t kernel_rsp;
    struct mm_struct *mm;  /* virtual memory info, kernel tasks use a global */
    struct task_struct *next_task, *prev_task;  /* for traversing all tasks */
    struct task_struct *next_rq,  *prev_rq;  /* for traversing a run queue */
};

enum task_flags {
    TASK_FIRST_SWITCH = 1
};

enum task_type {
    TASK_KERN,  /* kernel thread, uses another tasks mm_struct */
    TASK_USER   /* user thread */
};

enum task_state {
    TASK_UNRUNNABLE, /* unable to be scheduled */
    TASK_RUNNABLE,   /* able to be scheduled */
    TASK_DEAD,       /* after a context-switch the task is cleaned-up, set on exit() */
    TASK_BLOCKED     /* waiting for I/O */
};

/* Run-Queue */
struct rq {
    ulong num_switches; /* num context switches */
    struct task_struct *tasks; /* task queue */
};

void schedule(void);
void scheduler_init(void);
struct task_struct *ktask_create(void (*start)(void));

#ifdef WE_ARE_LINUX
struct task_struct {
/* these are hardcoded - don't touch */
    volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
    long counter;
    long priority;
    unsigned long signal;
    unsigned long blocked;	/* bitmap of masked signals */
    unsigned long flags;	/* per process flags, defined below */
    int errno;
    long debugreg[8];  /* Hardware debugging registers */
    struct exec_domain *exec_domain;
/* various fields */
    struct linux_binfmt *binfmt;
    struct task_struct *next_task, *prev_task;
    struct task_struct *next_run,  *prev_run;
    unsigned long saved_kernel_stack;
    unsigned long kernel_stack_page;
    int exit_code, exit_signal;
    /* ??? */
    unsigned long personality;
    int dumpable:1;
    int did_exec:1;
    /* shouldn't this be pid_t? */
    int pid;
    int pgrp;
    int tty_old_pgrp;
    int session;
    /* boolean value for session group leader */
    int leader;
    int	groups[NGROUPS];
    /*
     * pointers to (original) parent process, youngest child, younger sibling,
     * older sibling, respectively.  (p->father can be replaced with
     * p->p_pptr->pid)
     */
    struct task_struct *p_opptr, *p_pptr, *p_cptr, *p_ysptr, *p_osptr;
    struct wait_queue *wait_chldexit;	/* for wait4() */
    unsigned short uid,euid,suid,fsuid;
    unsigned short gid,egid,sgid,fsgid;
    unsigned long timeout, policy, rt_priority;
    unsigned long it_real_value, it_prof_value, it_virt_value;
    unsigned long it_real_incr, it_prof_incr, it_virt_incr;
    struct timer_list real_timer;
    long utime, stime, cutime, cstime, start_time;
/* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
    unsigned long min_flt, maj_flt, nswap, cmin_flt, cmaj_flt, cnswap;
    int swappable:1;
    unsigned long swap_address;
    unsigned long old_maj_flt;	/* old value of maj_flt */
    unsigned long dec_flt;		/* page fault count of the last time */
    unsigned long swap_cnt;		/* number of pages to swap on next pass */
/* limits */
    struct rlimit rlim[RLIM_NLIMITS];
    unsigned short used_math;
    char comm[16];
/* file system info */
    int link_count;
    struct tty_struct *tty; /* NULL if no tty */
/* ipc stuff */
    struct sem_undo *semundo;
    struct sem_queue *semsleeping;
/* ldt for this task - used by Wine.  If NULL, default_ldt is used */
    struct desc_struct *ldt;
/* tss for this task */
    struct thread_struct tss;
/* filesystem information */
    struct fs_struct *fs;
/* open file information */
    struct files_struct *files;
/* memory management info */
    struct mm_struct *mm;
/* signal handlers */
    struct signal_struct *sig;
};
#endif //WE_ARE_LINUX

#define switch_to(prev, next)                                              \
	__asm__ __volatile__ (                                                 \
         PUSHQALL                                                          \
	     "movq %%rsp, %P0(%1);"  /* save prev stack ptr */                 \
	     "movq %P0(%2), %%rsp;"  /* switch to next's stack */	           \
	     "testl %4, %P3(%2);"    /* check flags for TASK_FIRST_SWITCH */   \
	     "jz _normal_switch_to;"                                           \
	     "xorl %4, %P3(%2);"     /* clear TASK_FIRST_SWITCH flag */        \
	     "retq;"                 /* call task start function */            \
         "_normal_switch_to:;"                                             \
         POPQALL                                                           \
         : /* none */                                                      \
         : "i" (__builtin_offsetof(struct task_struct, kernel_rsp)),       \
	       "r" (prev), "r" (next),            	                           \
           "i" (__builtin_offsetof(struct task_struct, flags)),            \
           "i" (TASK_FIRST_SWITCH)                                         \
	     : "memory", "cc", "rax", "rcx", "rbx", "rdx", "r8", "r9", "r10",  \
           "r11", "r12", "r13", "r14", "r15", "flags")

#endif //_SBUNIX_SCHED_H
