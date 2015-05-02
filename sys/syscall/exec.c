#include <sbunix/sbunix.h>
#include <sbunix/tarfs.h>
#include <sbunix/sched.h>
#include <sbunix/elf64.h>
#include <sbunix/mm/vmm.h>
#include <errno.h>
#include <sbunix/string.h>

extern void printB(void);

int do_execve(char *filename, char *argv[], char *envp[]) {
    struct file *fp;
    struct mm_struct *mm;
    int err;
    uint64_t user_rsp, user_rip;
    /* TODO: validate user pointers */
    /* TODO: resolve filename to absolute path */
    fp = kmalloc(sizeof(struct file));
    if(!fp)
        return -ENOMEM;
    err = tarfs_open(filename, fp);
    if(err)
        goto cleanup_kmalloc;
    err = elf_validiate_exec(fp);
    if(err)
        goto cleanup_file;
    /* create new mm_struct */
    mm = mm_create();
    if(!mm) {
        err = -ENOMEM;
        goto cleanup_file;
    }
    err = elf_load(fp, mm);
    if(err)
        goto cleanup_mm;

    err = add_heap(mm);
    if(err)
        goto cleanup_mm;
    err = add_stack(mm, argv, envp);
    if(err)
        goto cleanup_mm;

    write_cr3(mm->pml4);
    /* If current task is a user, destroy it's mm_struct  */
    if(curr_task->type == TASK_KERN) {
        curr_task->type = TASK_USER;
    } else {
        /* free old mm_struct */
        mm_destroy(curr_task->mm);
    }
    curr_task->mm = mm;
    /* Update curr_task->cmdline  */
    strncpy(curr_task->cmdline, filename, TASK_CMDLINE_MAX);
    /* TODO: Copy on write stuff????? */
    user_rsp = mm->user_rsp;
    user_rip = mm->user_rip;
    debug("new mm->usr_rsp=%p, mm->user_rip=%p\n", user_rsp, user_rip);
//    Trying to use sysret to enter user space, but the rsp gets truncated, WHY???
//    __asm__ __volatile__(
//        "movq %0, %%rsp;"
//        "movq %1, %%rcx;"
//        "sysret;"
//        :  /* No output */
//        : "r"(user_rsp), "r"(user_rip)
//        :"memory"
//    );
    __asm__ __volatile__(
        "cli;"
        "movq $0x10, %%rax;"
        "movq %%rax, %%ds;"
        "movq %%rax, %%es;"
        "movq %%rax, %%fs;"
        "movq %%rax, %%gs;"
        "pushq %%rax;"         /* ring3 ss, should be _USER_DS|RPL = 0x23 */
        "pushq %0;"            /* ring3 rsp */
        "pushfq;"              /* ring3 rflags */
        "popq %%rax;"
        "or $0x200, %%rax;"    /* Set the IF flag, for interrupts in ring3 */
        "pushq %%rax;"
        "pushq $0x08;"         /* ring3 cs, should be _USER_CS|RPL = 0x1B */
        "pushq %1;"            /* ring3 rip */
        "iretq;"
        : /* No output */
        : "r"(user_rsp), "r"(user_rip)
        :"memory", "rax"
    );

    return 0;
cleanup_mm:
    mm_destroy(mm);
cleanup_file:
    fp->f_op->close(fp);
    return err;
cleanup_kmalloc:
    kfree(fp);
    return err;
}