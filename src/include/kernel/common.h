#ifndef C_OS_COMMON_H
#define C_OS_COMMON_H
/* 	ChickenOS - common.h
 * 	Contains commonly used routines such as kprintf and PANIC, and macros such as UNUSED
 */

#ifndef NULL
#define NULL 0
#endif
#define UNUSED  __attribute__((unused))

static inline void kernel_halt()
{
	asm volatile ("hlt");
	while(1);
}
void kprintf(char *fmt, ...);

//fix this once I have a kernel printf
#include <kernel/console.h>
#include <stdio.h>
static inline void print_stack_trace ()
{
      uint32_t *ebp, *eip;
      asm volatile ("mov %%ebp, %0" : "=r" (ebp)); // Start with the current EBP value.
      while (ebp)
      {
        eip = ebp+1;
        printf ("[0x%x], ", *eip);
        ebp = (uint32_t*) *ebp;	
		if((uintptr_t)ebp < 0xC0000000)
			break;
      }
}
#define BACK(x) case x: ret = __builtin_return_address(x);break;
//horrible, horrible abuse of GCC's builtin return_address 
//funtion, needs to print to serial so output can be pasted
//into addr2line -e kernel.bin -a -f [BACKTRACE]
extern void kmain(uint32_t mbd, uint32_t magic);
extern int StartInHigherHalf;
static inline void backtrace(unsigned int level)
{
	void *ret = 0;
	printf("Backtrace:\n");
	for(unsigned int i = 0; i < level; i++)
	{
		switch(i)
		{
			BACK(0);
			BACK(1);
			BACK(2);
			BACK(3);
			BACK(4);
			BACK(5);
			BACK(6);
			BACK(7);
			BACK(8);
			BACK(9);
			BACK(10);
			default:
				printf("too many levels\n");
				return;
		}	
		printf("%i [%X] %x\n",i, ret, (StartInHigherHalf));
		if((uintptr_t)ret == (uintptr_t)(StartInHigherHalf + 0xc0000000))
			return;
	}

	printf("end\n");
}
#undef BACK

#define PANIC(x) panic(__FILE__, __LINE__, x);
static inline void panic(char *file, int line,char* msg)
{
	console_set_color(RED,WHITE);
	console_puts("KERNEL PANIC\n");
	printf("file:%s line:%i [%s]\n",file, line, msg);
	printf("Backtrace\n");
	print_stack_trace();

	kernel_halt();
}
#define ASSERT(condition, msg) assert(__FILE__, __LINE__,condition, msg)
static inline void
assert(char *file, int line, bool condition, char *msg)
{
	console_set_color(RED,WHITE);
	
	if(condition == true)
		return;
	console_puts("Conditon failed:\t");
	console_puts(msg);
	
	panic(file, line, "ASSERTION FAILED!");
}
#endif
