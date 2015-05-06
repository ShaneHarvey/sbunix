#include <sbunix/interrupt/idt.h>
#include <sbunix/sbunix.h>

#define SAVEALL \
        "pushq %rax;" \
        "pushq %rbx;" \
        "pushq %rcx;" \
        "pushq %rdx;" \
        "pushq %rsi;" \
        "pushq %rdi;" \
        "pushq %r8;"  \
        "pushq %r9;"  \
        "pushq %r10;" \
        "pushq %r11;" \
        "pushq %r12;" \
        "pushq %r13;" \
        "pushq %r14;" \
        "pushq %r15;" \
        "pushq %rbp;"


#define RESTOREALL \
        "popq %rbp;" \
        "popq %r15;" \
        "popq %r14;" \
        "popq %r13;" \
        "popq %r12;" \
        "popq %r11;" \
        "popq %r10;" \
        "popq %r9;"  \
        "popq %r8;"  \
        "popq %rdi;" \
        "popq %rsi;" \
        "popq %rdx;" \
        "popq %rcx;" \
        "popq %rbx;" \
        "popq %rax;"

#define DEBUG_IRETQ_NO_ERROR_CODE \
    "movq 120(%rsp), %rdi;"  /* 1st arg: faulting instruction pointer */ \
    "movq 128(%rsp), %rsi;"  /* 2nd arg: iretq CS */                     \
    "movq 136(%rsp), %rdx;"  /* 3rd arg: iretq RFLAGS */                 \
    "movq 144(%rsp), %rcx;"  /* 4th arg: iretq RIP */                    \
    "movq 152(%rsp), %r8;"   /* 5th arg: iretq SS */                     \
    "call debug_iretq;"

#define DEBUG_IRETQ_WITH_ERROR_CODE \
    "movq 128(%rsp), %rdi;"  /* 1st arg: faulting instruction pointer */ \
    "movq 136(%rsp), %rsi;"  /* 2nd arg: iretq CS */                     \
    "movq 144(%rsp), %rdx;"  /* 3rd arg: iretq RFLAGS */                 \
    "movq 152(%rsp), %rcx;"  /* 4th arg: iretq RIP */                    \
    "movq 160(%rsp), %r8;"   /* 5th arg: iretq SS */                     \
    "call debug_iretq;"


#define ISR_WRAPPER(vector)                                              \
    __asm__ (                                                            \
        ".global _isr_wrapper_" # vector "\n"                            \
        "_isr_wrapper_" # vector ":\n"                                   \
            SAVEALL                                                     \
            /* DEBUG_IRETQ_NO_ERROR_CODE */                                 \
            "call _isr_handler_" # vector ";"                            \
            RESTOREALL                                                      \
            "iretq;" )

#define ISR_WRAPPER_ERROR_CODE(vector)                                   \
    __asm__ (                                                            \
        ".global _isr_wrapper_" # vector "\n"                            \
        "_isr_wrapper_" # vector ":\n"                                   \
            SAVEALL                                                     \
            DEBUG_IRETQ_WITH_ERROR_CODE                                  \
            "call _isr_handler_" # vector ";"                            \
            RESTOREALL                                                      \
            "iretq;" )


void debug_iretq(uint64_t fault_rip, uint64_t cs, uint64_t rflags, uint64_t rsp, uint64_t ss) {
    debug("CURR_RSP:%p\nIRETQ: RSP:%p RIP:%p SS:%p RFLAGS:%p CS:%p\n", read_rsp(),
          (void*)rsp, (void*)fault_rip, (void*)ss, (void*)rflags, (void*)cs);
}

/* Intel Reserved 0-31 */
ISR_WRAPPER(0);
ISR_WRAPPER(1);
ISR_WRAPPER(2);
ISR_WRAPPER(3);
ISR_WRAPPER(4);
ISR_WRAPPER(5);
ISR_WRAPPER(6);
ISR_WRAPPER(7);
ISR_WRAPPER_ERROR_CODE(8);
ISR_WRAPPER(9);
ISR_WRAPPER_ERROR_CODE(10);
ISR_WRAPPER_ERROR_CODE(11);
ISR_WRAPPER_ERROR_CODE(12);

/** Special General Protection Fault wrapper
* After SAVEALL,
* 120(%rsp)  is  error code
* 128(%rsp)  is  faulting instruction pointer
*/
__asm__ (
".global _isr_wrapper_13\n"
        "_isr_wrapper_13:\n"
        SAVEALL
        DEBUG_IRETQ_WITH_ERROR_CODE
        "movq 120(%rsp), %rdi;" /* 1st arg: Error code into %rdi. */
        "movq 128(%rsp), %rsi;" /* 2nd arg: faulting instruction pointer %rsi. */
        "call _isr_handler_13;"
        RESTOREALL
        "addq $0x8, %rsp;"      /* MUST POP errorcode */
        "iretq;"
);

/** Special Page Fault wrapper
* After SAVEALL,
* cr2 holds faulting address
* 120(%rsp)  is  error code
*
* Grab the faulting address from %cr2
*/
__asm__ (
    ".global _isr_wrapper_14\n"
    "_isr_wrapper_14:\n"
        SAVEALL
        DEBUG_IRETQ_WITH_ERROR_CODE
        "movq 120(%rsp), %rdi;"  /* 1st arg: Error code into %rsi. */
        "movq 128(%rsp), %rsi;"  /* 2nd arg: faulting instruction pointer */
        "call _isr_handler_14;"
        RESTOREALL
        "addq $0x8, %rsp;"      /* MUST POP errorcode */
        "iretq;"
);

ISR_WRAPPER(15);
ISR_WRAPPER(16);
ISR_WRAPPER_ERROR_CODE(17);
ISR_WRAPPER(18);
ISR_WRAPPER(19);
ISR_WRAPPER(20);
ISR_WRAPPER(21);
ISR_WRAPPER(22);
ISR_WRAPPER(23);
ISR_WRAPPER(24);
ISR_WRAPPER(25);
ISR_WRAPPER(26);
ISR_WRAPPER(27);
ISR_WRAPPER(28);
ISR_WRAPPER(29);
ISR_WRAPPER_ERROR_CODE(30);
ISR_WRAPPER(31);

/* The PIC IRQ's */
ISR_WRAPPER(32);    /* Programmable Interrupt Timer Interrupt */
ISR_WRAPPER(33);    /* Keyboard Interrupt */
ISR_WRAPPER(34);    /* Cascade (used internally by the two PICs. never raised) */
ISR_WRAPPER(35);    /* COM2 (if enabled) */
ISR_WRAPPER(36);    /* COM1 (if enabled) */
ISR_WRAPPER(37);    /* LPT2 (if enabled) */
ISR_WRAPPER(38);    /* Floppy Disk */
ISR_WRAPPER(39);    /* LPT1 / Unreliable "spurious" interrupt (usually) */
ISR_WRAPPER(40);    /* CMOS real-time clock (if enabled) */
ISR_WRAPPER(41);    /* Free for peripherals / legacy SCSI / NIC */
ISR_WRAPPER(42);    /* Free for peripherals / SCSI / NIC */
ISR_WRAPPER(43);    /* Free for peripherals / SCSI / NIC */
ISR_WRAPPER(44);    /* PS2 Mouse */
ISR_WRAPPER(45);    /* FPU / Coprocessor / Inter-processor */
ISR_WRAPPER(46);    /* Primary ATA Hard Disk */
ISR_WRAPPER(47);    /* Secondary ATA Hard Disk */
