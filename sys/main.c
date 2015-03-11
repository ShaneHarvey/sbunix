#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/tarfs.h>
#include <sys/pic8259.h>
#include <sys/pit.h>

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
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			printf("Available Physical Memory [%lx-%lx]\n", smap->base, smap->base + smap->length);
		}
	}
	printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	printf("physbase %p, physfree %p\n", physbase, physfree);
	/* kernel starts here */
	load_idt();
	PIC_protected_mode();
	pit_set_freq(18.0);
	while(1){
		/* do nothing */
	}

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
	printf("!!!!! start() returned !!!!!");
	while(1){
		__asm__ __volatile__ ("hlt;");
	}
}
