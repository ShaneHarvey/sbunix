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
    debug("new mm->usr_rsp=%p, mm->user_rip=%p\n", mm->user_rsp, mm->user_rip);
    __asm__ __volatile__(
        "movq %%rsp, %%rbx;"
        "pushq $0x10;"       /* ss */
        "pushq %%rbx;"       /* rsp */
        "pushfq;"            /* rflags */
        "pushq $0x08;"       /* cs */
        "pushq %1;"     /* rip */
        "iretq;"
        : /* No output */
        : "r"(mm->user_rsp), "r"(mm->user_rip)
        :"memory"
    );
//    __asm__ __volatile__(
//        "cli;"
//        "mov $0x23, %%ax;"
//        "mov %%ax, %%ds;"
//        "mov %%ax, %%es;"
//        "mov %%ax, %%fs;"
//        "mov %%ax, %%gs;"
//        "pushq $0x23;"
//        "pushq %0;"    /* Push user stack pointer */
//        "pushfq;"
////        "popq %%rax;"       // Get EFLAGS back into EAX. The only way to read EFLAGS is to pushf then pop.
////        "or $0x200, %%rax;" // Set the IF flag.
////        "pushq %%rax;"      // Push the new EFLAGS value back onto the stack.
//        "pushq $0x1B;"
//        "pushq %1;"    /* Push user instruction pointer */
//        "swapgs;"
//        "iretq;"
//        : /* No output */
//        : "r"(mm->user_rsp), "r"(mm->user_rip)
//        :"memory"
//    );

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