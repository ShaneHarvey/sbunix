#include <sbunix/sbunix.h>
#include <sbunix/gdt.h>
#include <sbunix/tarfs.h>
#include <sbunix/interrupt/idt.h>
#include <sbunix/interrupt/pic8259.h>
#include <sbunix/interrupt/pit.h>
#include <sbunix/mm/physmem.h>

void test_scroll(void) {
	int i = 0;
	for(i = 0; i < 40; i++)
		printf("scroll test: %d\n", i);
	printf("This is a 78 character line---------------------------------------------------\n");
	printf("This is a 79 character line----------------------------------------------------\n");
	printf("This is a 80 character line-----------------------------------------------------\n");
	printf("This is a 81 character line------------------------------------------------------\n");
	printf("This is a 82 character line-------------------------------------------------------\n");
	printf("This is a tab:'\t'-------------------------------------------------------------\n");
	printf("This is a space:' '------------------------------------------------------------\n");
}

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	struct pzone *pzonehd = NULL;
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) {
		modulep += modulep[1] + 2;
	}
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			printf("Available Physical Memory [%lx-%lx]\n", smap->base, smap->base + smap->length);
			pzonehd = pzone_new(smap->base, smap->base + smap->length, PZONE_USABLE);
		}
	}
	/**
	* This is between physbase and physfree:
	* physbase				= 0x200000
	* _binary_tarfs_start 	= 0x209780
	* _binary_tarfs_end 	= 0x32df80
	* physfree				= 0x333000
	* The kernel's size (physfree) will grow but is tarfs in a fixed location?
	* How should we treat this memory range?
	*/
	printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	printf("physbase %p, physfree %p\n", physbase, physfree);

	pzone_remove(0x3f000, 0x42000); /* FIXME: DELETE line after moving page tables!*/

	pzonehd = pzone_remove((uint64_t)physbase, (uint64_t)physfree);

	/* kernel starts here */
	load_idt();
	PIC_protected_mode();
	pit_set_freq(18.0);

	/* Physical Mem Init */
	physmem_init(pzonehd);
	physmem_report();

	halt_loop("Halting in start(), time and key presses should update...\n");

	//test_scroll();
	//int x = 8;
	//int y = 15;
	//printf("Div by zero %d\n", x/(y-15));
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase; /* defined by linker.script */
struct tss_t tss;

/**
* boot is the entry point to the kernel:
* ENTRY(boot) in the linker.script
*/
void boot(void)
{
	/* note: function changes rsp, local stack variables can't be practically used */
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);
	reload_gdt();
	setup_tss();
	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	halt_loop("!!!!! start() returned !!!!!\n");
}
