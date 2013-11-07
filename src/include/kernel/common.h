#ifndef C_OS_COMMON_H
#define C_OS_COMMON_H
/* 	ChickenOS - common.h
 * 	Contains commonly used routines such as kprintf and PANIC, and macros such as UNUSED
 */
#include <stdbool.h>

//fix this once I have a kernel printf
#include <device/console.h>
#include <stdio.h>

#ifdef ARCH_ARM
#include <arch/arm/common.h>
#elif ARCH_I386
#include <arch/i386/common.h>
#endif

#ifndef NULL
#define NULL 0
#endif

#define UNUSED  __attribute__((unused))

void kprintf(char *fmt, ...);

#define BACK(x) case x: ret = __builtin_return_address(x);break;
//horrible, horrible abuse of GCC's builtin return_address 
//funtion, needs to print to serial so output can be pasted
//into addr2line -e kernel.bin -a -f [BACKTRACE]
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
		if((uintptr_t)ret == (uintptr_t)((StartInHigherHalf& 0xFFF) + 0xc0000000))
			return;
	}

	printf("end\n");
}
#undef BACK

//FIXME: Get colored coded messages fixed
#define PANIC(x) panic(__FILE__, __LINE__, x);
static inline void panic(char *file, int line,char* msg)
{
	//console_set_color(RED,WHITE);
	printf("KERNEL PANIC\n");
	printf("file:%s line:%i [%s]\n",file, line, msg);
	printf("Backtrace\n");
	print_stack_trace();
	kernel_halt();
}
#define ASSERT(condition, msg) kassert(__FILE__, __LINE__,condition, msg, __func__)
static inline void
kassert(char *file, int line, bool condition, char *msg, const char *function)
{
	if(condition == true)
		return;
	
	//console_set_color(GREEN,WHITE);
	printf("%s(): ",function);
	printf("%s\n",msg);
		
	panic(file, line, "ASSERTION FAILED!");
}
#endif
