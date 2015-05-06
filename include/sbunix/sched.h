#ifndef _SBUNIX_SCHED_H
#define _SBUNIX_SCHED_H

#include <sys/defs.h>
#include <sbunix/mm/types.h> /* mm_struct */
#include <sbunix/fs/vfs.h>   /* file */
#include <sbunix/time.h>

extern struct queue         run_queue;
extern struct queue         just_ran_queue;
extern struct queue         block_queue;
extern struct mm_struct     kernel_mm;
extern struct task_struct   kernel_task;
extern struct task_struct   *curr_task;

#define TASK_CMDLINE_MAX 128
#define TASK_FILES_MAX   64

/* Kernel thread or user process */
struct task_struct {
    int type;
    int state;
    int first_switch;
    int foreground;       /* True if this task controls the terminal */
    int in_syscall;       /* Set to 1 if this task is in a system call */
    struct timespec sleepts; /* time left to sleep */
    uint64_t kernel_rsp;
    pid_t pid;            /* Process ID, monotonically increasing. 0 is not valid */
    int exit_code;        /* Exit code of a process, returned by wait() */
    struct mm_struct *mm; /* virtual memory info, kernel tasks all share a global */
    struct task_struct *next_task, *prev_task; /* for traversing all tasks */
    struct task_struct *next_rq;    /* for traversing a run queue */
    struct task_struct *parent, *chld, *sib;   /* parent/child/sibling pointers */
    struct file *files[TASK_FILES_MAX];
    char cmdline[TASK_CMDLINE_MAX + 1];
};

enum task_type {
    TASK_KERN = 1, /* kernel thread, uses another tasks mm_struct */
    TASK_USER = 2  /* user thread */
};

enum task_state {
    TASK_UNRUNNABLE = 1,  /* unable to be scheduled */
    TASK_RUNNABLE   = 2,  /* able to be scheduled */
    TASK_DEAD       = 4,  /* after a context-switch the task is cleaned-up, set on exit() */
    TASK_BLOCKED    = 8,  /* waiting for I/O */
    TASK_SLEEPING   = 16  /* sleeping */
};

/* Run-Queue */
struct queue {
    ulong num_switches; /* num context switches */
    struct task_struct *tasks; /* task queue */
};

void task_block(void);
void task_unblock_foreground(void);
int task_sleeping(struct task_struct *task);
void schedule(void);
void scheduler_init(void);
struct task_struct *ktask_create(void (*start)(void), char *name);
void task_set_cmdline(struct task_struct *task, char *cmdline);
pid_t get_next_pid(void);
void scheduler_test(void);
void debug_task(struct task_struct *task);
void reset_timeslice(struct task_struct *task);
void kill_curr_task(int exit_code);
struct task_struct *fork_curr_task(void);
int task_files_init(struct task_struct *task);

#define switch_to(prev, next)                                              \
	__asm__ __volatile__ (                                                 \
         PUSHQALL                                                          \
	     "movq %%rsp, %P0(%1);"  /* save prev stack ptr */                 \
	     "movq %P0(%2), %%rsp;"  /* switch to next's stack */	           \
	     "cmpl $0x0, %P3(%2);"    /* check flags for first_switch */   \
	     "jne _skip_popqall;"    /* If first switch skip the pops */       \
         POPQALL                                                           \
         "_skip_popqall:;"                                                 \
         : /* none */                                                      \
         : "i" (__builtin_offsetof(struct task_struct, kernel_rsp)),       \
	       "r" (prev),                                                     \
           "r" (next),            	                                       \
           "i" (__builtin_offsetof(struct task_struct, first_switch))      \
	     : "memory", "cc", "rax", "rcx", "rbx", "rdx", "r8", "r9", "r10",  \
           "r11", "r12", "r13", "r14", "r15", "flags")

#endif //_SBUNIX_SCHED_H
