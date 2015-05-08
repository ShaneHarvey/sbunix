#ifndef _SBUNIX_SCHED_H
#define _SBUNIX_SCHED_H

#include <sys/defs.h>
#include <sys/signal.h>
#include <sbunix/mm/types.h> /* mm_struct */
#include <sbunix/fs/vfs.h>   /* file */
#include <sbunix/time.h>

/* For description of queues see sched.c */
extern struct queue run_queue;
extern struct queue just_ran_queue;
extern struct queue block_queue;
extern struct queue sleep_queue;
extern struct queue wait_queue;

/* These two are the kernel's, after exec'ing /bin/init these describe
 * the idle task, because kmain just calls halt. */
extern struct mm_struct     kernel_mm;
extern struct task_struct   kernel_task;

/* The init task is created in kmain, it is PID 1 */
extern struct task_struct   *init_task;

/* The current task that we are executing */
extern struct task_struct   *curr_task;

#define TASK_CMDLINE_MAX 128
#define TASK_FILES_MAX   64
#define TASK_CWD_MAX   99

/* Kernel thread or user process */
struct task_struct {
    int type;             /* See enum task_type */
    int state;            /* See enum task_state */
    int first_switch;     /* Flag for tricks to kick start a task */
    int foreground;       /* True if this task controls the terminal */
    int in_syscall;       /* Set to 1 if this task is in a system call */
    int timeslice;        /* User timeslices */
    struct timespec sleepts; /* time left to sleep */
    pid_t pid;            /* Process ID, monotonically increasing. 0 is not valid */
    int exit_code;        /* Exit code of a process, returned by wait() */
    void *blocked_on;     /* Pointer to data structure that this task is waiting on */
    uint64_t kernel_rsp;  /* Kernel's 4KB stack */
    struct mm_struct *mm; /* virtual memory info, kernel tasks all share &kernel_mm */
    struct task_struct *next_task, *prev_task; /* for traversing all tasks */
    struct task_struct *next_rq;               /* for traversing a queue */
    struct task_struct *parent, *chld, *sib;   /* parent/child/sibling pointers */
    struct file *files[TASK_FILES_MAX];
    char cmdline[TASK_CMDLINE_MAX + 1];
    char cwd[TASK_CWD_MAX + 1];
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
    TASK_SLEEPING   = 16, /* sleeping */
    TASK_WAITING    = 32  /* waiting on children */
};

/* Exit Codes */
#define EXIT_FATALSIG   128  /* Added to fatal signal for exit codes */

/* Base timeslice in number of interrupts */
#define TIMESLICE_BASE  80

/* Run-Queue */
struct queue {
    ulong num_switches; /* num context switches */
    struct task_struct *tasks; /* task queue */
};

struct task_struct *foreground_task(void);
void task_block(void *block_on);
void task_wakeup(struct queue *from_queue, struct task_struct *task);
void task_unblock(void *blocked_on);
void task_unblock_foreground(void *blocked_on);
void schedule(void);
void scheduler_init(void);
struct task_struct *ktask_create(void (*start)(void), const char *name);
void task_set_cmdline(struct task_struct *task, const char *cmdline);
pid_t get_next_pid(void);
void debug_task(struct task_struct *task);
struct task_struct *fork_curr_task(void);
int task_files_init(struct task_struct *task);
int cleanup_child(struct task_struct *task);
void kill_curr_task(int exit_code);
void kill_other_task(struct task_struct *task, int exit_code);
void send_signal(struct task_struct *task, int signal);

static inline void reset_timeslice(struct task_struct *task) {
    task->timeslice = TIMESLICE_BASE;
}

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
