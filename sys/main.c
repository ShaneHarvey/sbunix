#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>

void test_scroll(void) {
	int i = 0;
	for(i = 0; i < 100; i++)
		printf("scroll test: %d\n", i);
	printf("This is a 78 character line---------------------------------------------------\n");
	printf("This is a 79 character line----------------------------------------------------\n");
	printf("This is a 80 character line-----------------------------------------------------\n");
	printf("This is a 81 character line------------------------------------------------------\n");
	printf("This is a 82 character line-------------------------------------------------------\n");
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
			printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
	}
	printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	// kernel starts here
	test_scroll();
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase; /* defined by linker.script */
struct tss_t tss;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
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
	__asm__ ("hlt;");
	while(1);
}
