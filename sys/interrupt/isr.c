#define ISR_WRAPPER(vector) \
    __asm__ ( \
        ".global _isr_wrapper_" # vector "\n" \
        "_isr_wrapper_" # vector ":\n" \
        "    pushq %rax;" \
        "    pushq %rbx;" \
        "    pushq %rcx;" \
        "    pushq %rdx;" \
        "    pushq %rsi;" \
        "    pushq %rdi;" \
        "    pushq %rbp;" \
        "    pushq %r8;" \
        "    pushq %r9;" \
        "    pushq %r10;" \
        "    pushq %r11;" \
        "    pushq %r12;" \
        "    pushq %r13;" \
        "    pushq %r14;" \
        "    pushq %r15;" \
        "    call _isr_handler_" # vector ";" \
        "    popq %r15;" \
        "    popq %r14;" \
        "    popq %r13;" \
        "    popq %r12;" \
        "    popq %r11;" \
        "    popq %r10;" \
        "    popq %r9;" \
        "    popq %r8;" \
        "    popq %rbp;" \
        "    popq %rdi;" \
        "    popq %rsi;" \
        "    popq %rdx;" \
        "    popq %rcx;" \
        "    popq %rbx;" \
        "    popq %rax;" \
        "    iretq;" )


ISR_WRAPPER(0);
ISR_WRAPPER(1);
ISR_WRAPPER(2);
ISR_WRAPPER(3);
ISR_WRAPPER(4);
ISR_WRAPPER(5);
ISR_WRAPPER(6);
ISR_WRAPPER(7);
ISR_WRAPPER(8);
ISR_WRAPPER(9);
ISR_WRAPPER(10);
ISR_WRAPPER(11);
ISR_WRAPPER(12);
ISR_WRAPPER(13);
ISR_WRAPPER(14);
ISR_WRAPPER(15);
ISR_WRAPPER(16);
ISR_WRAPPER(17);
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
ISR_WRAPPER(30);
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
