#include <sbunix/sbunix.h>
#include <sbunix/gdt.h>
#include <sbunix/fs/tarfs.h>
#include <sbunix/interrupt/idt.h>
#include <sbunix/interrupt/pic8259.h>
#include <sbunix/interrupt/pit.h>
#include <sbunix/mm/physmem.h>
#include <sbunix/mm/pt.h>
#include <sbunix/sched.h>
#include "kmain.h"
#include "syscall/syscall_dispatch.h"


/*
 * This is the base virtual address that maps one-to-one to physical memory
 * It is defined by kernmem - physbase (see /linker.script)
 */
uint64_t virt_base;

void start(uint32_t* modulep, uint64_t physbase, uint64_t physfree)
{
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) {
		modulep += modulep[1] + 2;
	}
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			printk("Available Physical Memory [%lx-%lx]\n", smap->base, smap->base + smap->length);
			pzone_new(smap->base, smap->base + smap->length, PZONE_USABLE);
		}
	}

	printk("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	printk("physbase 0x%lx, physfree 0x%lx\n", physbase, physfree);

	/* Initialize interrupts and memory allocation */
	load_idt();
	PIC_protected_mode();
	init_unix_time();
	pit_set_freq(1000);  /* 1000 HZ (1 millisecond) (1000000 nanoseconds) */

	/* Init kernel page table */
	init_kernel_pt(physfree);
	physfree += 3 * PAGE_SIZE; /* Kernel Page Tables took 3 pages */

	/* Init physical memory tracking */
	pzone_remove(physbase, physfree);  /* Remove kernel's code and data */
	physmem_init();
	physmem_report();

	tarfs_init();

	scheduler_init();

	enable_syscalls();
	/* Start the kernel */
	kmain();
	halt_loop("Halting in start(), time and key presses should update...\n");
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase; /* defined by linker.script */

/**
* boot is the entry point to the kernel:
* ENTRY(boot) in the linker.script
*/
void boot(void)
{
	/* note: function changes rsp, local stack variables can't be practically used */
	__asm__ __volatile__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE - 16])
	);
	reload_gdt();
	setup_tss();
	virt_base = (uint64_t)&kernmem - (uint64_t)&physbase;
	start((uint32_t*)(loader_stack[3] + virt_base), (uint64_t)&physbase,
		  (uint64_t)loader_stack[4]);
	halt_loop("!!!!! start() returned !!!!!\n");
}
