#include <sbunix/sbunix.h>
#include <sbunix/fs/tarfs.h>
#include <sbunix/fs/elf64.h>
#include <sbunix/mm/vmm.h>
#include <sbunix/gdt.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>
#include <sbunix/syscall.h>
#include <errno.h>
#include <ctype.h>
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

char *is_interpreter(struct file *fp) {
    struct posix_header_ustar *hdr;
    char *file_start;

    hdr = fp->private_data;
    /* Hack: we know it's a tarfs file in memory, so just get the data pointer */
    file_start = (char *)(hdr + 1);
    if(file_start[0] == '#' &&  file_start[1] == '!')
        return file_start + 2;
    return NULL;
}

/**
 * TODO: MUST support #! scripts
 * "./script.sh" -->> "/bin/sbush script.sh"
 */
static char inter_prev = 0;
//static int execcount = 0;
long do_execve(char *filename, const char **argv, const char **envp, int rec) {
    struct file *fp;
    struct mm_struct *mm;
    char *rpath;
    int ierr;
    long err;
    const char **copyargv = NULL;
    char *inter;
    size_t len;

//    printk("exec rec = %d, filename= %s\n", rec, filename);

    /* Resolve pathname to an absolute path */
    rpath = resolve_path(curr_task->cwd, filename, &err);
    if(!rpath)
        goto cleanup_rec_argv;

    if(rec == 1) {
        len = strlen(filename);
        filename[len] = inter_prev;
    }

    fp = tarfs_open(rpath, O_RDONLY, 0, &ierr);
    kfree(rpath);
    if(ierr) {
        err = ierr;
        goto cleanup_rec_argv;
    }

    if(rec == 0 && (inter = is_interpreter(fp))) {
        /* filename needs to be launched as a exec(interpreter, argv, evnp) */
        int i;
        for(i = 0; !isspace(inter[i]) && inter[i] != '\0'; i++); /*nothing*/;
        inter_prev = inter[i];
        inter[i] = '\0';
        copyargv = (const char **)get_free_page(0);
        if(!copyargv) {
            err = -ENOMEM;
            goto cleanup_file;
        }
        copyargv[0] = inter;
        /* copy argv */
        for(i = 1; argv[i - 1] && i < 200 ; i++)
            copyargv[i] = argv[i - 1];
        copyargv[i] = NULL;
        if(i >= 200) {
            err = -E2BIG;
            goto cleanup_copyargs;
        }
        fp->f_op->close(fp);
        return do_execve(inter, copyargv, envp, 1);
    }

    err = elf_validiate_exec(fp);
    if(err)
        goto cleanup_copyargs;
    /* create new mm_struct */
    mm = mm_create();
    if(!mm) {
        err = -ENOMEM;
        goto cleanup_copyargs;
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

    if(rec == 1)
        free_page((uint64_t)argv);

    /* Update curr_task->cmdline  */
    task_set_cmdline(curr_task, filename);

    write_cr3(mm->pml4);
    /* If current task is a user, destroy it's mm_struct  */
    if(curr_task->type == TASK_KERN) {
        curr_task->type = TASK_USER;
    } else {
        /* free old mm_struct */
        mm_destroy(curr_task->mm);
    }
    curr_task->mm = mm;
    fp->f_op->close(fp);
    debug("new mm->usr_rsp=%p, mm->user_rip=%p\n", mm->user_rsp, mm->user_rip);
//    if(execcount++ > 3)
//        return -ENOEXEC;
    enter_usermode(mm->user_rsp, mm->user_rip);
    /* cannot return here */
    return -ENOEXEC;
cleanup_mm:
    printk("do_exec: mm_destroy(mm=%p)\n", mm);
    mm_destroy(mm);
cleanup_copyargs:
    if(copyargv) {
        printk("do_exec: free_page(copyargv=%p)\n", copyargv);
        free_page((uint64_t)copyargv);
    }
cleanup_file:
    printk("do_exec: close(fp=%p)\n", fp);
    fp->f_op->close(fp);
cleanup_rec_argv:
    if(rec == 1) {
        printk("do_exec: free_page(argv=%p)\n", argv);
        free_page((uint64_t)argv);
    }
    return err;
}