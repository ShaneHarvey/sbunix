#ifndef _SBUNIX_ASM_H
#define _SBUNIX_ASM_H

/* Header for common assembly routines. */

#define PUSHQALL \
        "pushq %%rax;" \
        "pushq %%rbx;" \
        "pushq %%rcx;" \
        "pushq %%rdx;" \
        "pushq %%rsi;" \
        "pushq %%rdi;" \
        "pushq %%r8;"  \
        "pushq %%r9;"  \
        "pushq %%r10;" \
        "pushq %%r11;" \
        "pushq %%r12;" \
        "pushq %%r13;" \
        "pushq %%r14;" \
        "pushq %%r15;" \
        "pushq %%rbp;"


#define POPQALL \
        "popq %%rbp;" \
        "popq %%r15;" \
        "popq %%r14;" \
        "popq %%r13;" \
        "popq %%r12;" \
        "popq %%r11;" \
        "popq %%r10;" \
        "popq %%r9;"  \
        "popq %%r8;"  \
        "popq %%rdi;" \
        "popq %%rsi;" \
        "popq %%rdx;" \
        "popq %%rcx;" \
        "popq %%rbx;" \
        "popq %%rax;"

#define halt_loop(fmt, ...) \
            do { \
                printf(fmt, ##__VA_ARGS__); \
                while(1) { \
                    __asm__ __volatile__ ("hlt;"); \
                } \
            } while(0)

/**
 * Read the current value of the stack pointer
 */
static inline uint64_t read_rsp(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(ret));
    return ret;
}

/**
 * Read a 64-bit value from a MSR. The A constraint stands for concatenation
 * of registers EAX and EDX.
 */
static inline uint64_t rdmsr(uint32_t msr_id) {
    uint64_t msr_value;
    __asm__ __volatile__ ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
    return msr_value;
}

/**
 * Write a 64-bit value to a MSR. The A constraint stands for concatenation
 * of registers EAX and EDX.
 */
static inline void wrmsr(uint32_t msr_id, uint64_t msr_value) {
    __asm__ __volatile__ ( "wrmsr" : : "c" (msr_id), "A" (msr_value) );
}

static inline uint64_t read_cr0(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr0, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr1(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr1, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr3(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr3, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr4(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr4, %0;":"=r"(ret));
    return ret;
}

static inline uint64_t read_cr8(void) {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%cr8, %0;":"=r"(ret));
    return ret;
}

static inline void write_cr3(uint64_t pml4e_ptr) {
    __asm__ __volatile__ ("movq %0, %%cr3;"::"r"(pml4e_ptr));
}

#define switch_to_last(prev, next, last)                                       \
	__asm__ __volatile__ (                                                \
         /*PUSHQALL*/                                                     \
         "pushq %1;"            /* ptr to prev on my stack */             \
         "pushq %3;"            /* ptr to local last (&last) */           \
	     "movq %%rsp, %P0(%1);" /* save prev stack ptr */                 \
	     "movq %P0(%2), %%rsp;" /* switch to next's stack */	          \
         "popq %3;"             /* get next’s ptr to &last */             \
         "movq %1, (%3);"       /* store prev in &last */                 \
         "popq %1;"             /* Update me (rax) to new task */         \
         /*POPQALL*/                                                      \
         : /* none */                                                     \
         : "i" (__builtin_offsetof(struct task_struct, kernel_rsp)),      \
	       "r" (prev), "r" (next), "r" (last)	                          \
	     : "memory", "cc", "rax", "rcx", "rbx", "rdx", "r8", "r9", "r10", \
           "r11", "r12", "r13", "r14", "r15", "flags")

#define switch_to(prev, next)                                             \
	__asm__ __volatile__ (                                                \
         "pushq %1;"            /* ptr to prev on my stack */             \
         /*PUSHQALL*/                                                     \
	     "movq %%rsp, %P0(%1);" /* save prev stack ptr */                 \
	     "movq %P0(%2), %%rsp;" /* switch to next's stack */	          \
         "popq %1;"             /* Update me (rax) to new task */         \
         /*POPQALL*/                                                      \
         : /* none */                                                     \
         : "i" (__builtin_offsetof(struct task_struct, kernel_rsp)),      \
	       "r" (prev), "r" (next)            	                          \
	     : "memory", "cc", "rax", "rcx", "rbx", "rdx", "r8", "r9", "r10", \
           "r11", "r12", "r13", "r14", "r15", "flags")
#endif
