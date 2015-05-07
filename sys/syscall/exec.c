#include <sbunix/sbunix.h>
#include <sbunix/fs/tarfs.h>
#include <sbunix/fs/elf64.h>
#include <sbunix/mm/vmm.h>
#include <sbunix/gdt.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>
#include <sbunix/syscall.h>
#include <errno.h>
#include "syscall_dispatch.h"

void enter_usermode(uint64_t user_rsp, uint64_t user_rip) {
    cli();
    /* Same is in post_context_switch(), kernel stacks always aligned up minus 16 */
    tss.rsp0 = ALIGN_UP(read_rsp(), PAGE_SIZE) - 16;
    syscall_kernel_rsp = tss.rsp0;
    __asm__ __volatile__(
        "movq $0x23, %%rax;"
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
        "pushq $0x2B;"         /* ring3 cs, should be _USER64_CS|RPL = 0x2B */
        "pushq %1;"            /* ring3 rip */
        "xorq %%rax, %%rax;"   /* zero the user registers */
        "xorq %%rbx, %%rbx;"
        "xorq %%rcx, %%rcx;"
        "xorq %%rdx, %%rdx;"
        "xorq %%rbp, %%rbp;"
        "xorq %%rsi, %%rsi;"
        "xorq %%rdi, %%rdi;"
        "xorq %%r8, %%r8;"
        "xorq %%r9, %%r9;"
        "xorq %%r10, %%r10;"
        "xorq %%r11, %%r11;"
        "xorq %%r12, %%r12;"
        "xorq %%r13, %%r13;"
        "xorq %%r14, %%r14;"
        "xorq %%r15, %%r15;"
        "iretq;"
        : /* No output */
        : "r"(user_rsp), "r"(user_rip)
        :"memory", "rax"
    );
    kpanic("FAILED to enter user mode!");
}


/**
 * TODO: MUST support #! scripts
 * "./script.sh" -->> "/bin/sbush script.sh"
 */
int do_execve(const char *filename, const char **argv, const char **envp) {
    struct file *fp;
    struct mm_struct *mm;
    int err;

    /* TODO: resolve filename to absolute path */
    fp = tarfs_open(filename, 0, 0, &err);
    if(err)
        return err;
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
    task_set_cmdline(curr_task, filename);
    debug("new mm->usr_rsp=%p, mm->user_rip=%p\n", mm->user_rsp, mm->user_rip);
    enter_usermode(mm->user_rsp, mm->user_rip);
    return -ENOEXEC;
cleanup_mm:
    mm_destroy(mm);
cleanup_file:
    fp->f_op->close(fp);
    return err;
}