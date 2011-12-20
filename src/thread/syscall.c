#include <kernel/interrupt.h>
#include <kernel/console.h>
#include <kernel/thread.h>
#include <kernel/syscall.h>
#include <stdio.h>
//#include <kernel/vfs.h>

#define SYSCALL_0(num) ({				\
					int ret;			\
	asm volatile ( 	"pushl %1\n"		\
					"int $0x80\n"		\
				    "mov %%eax,%0\n"	\
					"addl $4, %%esp\n"	\
				   	: "=a"(ret) 		\
				   	: "i" (num)			\
					: "memory"			\
				   	);					\
					ret;})
int sys_dummy()
{
	return 0xcafebabe;	
}

int dummy()
{
	int test = SYSCALL_0(SYS_DUMMY);
	return test;
}


void syscall_handler (struct registers *regs)
{
	int call = (int)((int *)regs->useresp);
	switch (call)
	{
		case SYS_DUMMY:
			regs->eax = sys_dummy();
			return;
		default:
			printf("undefined system call!\n");
			return;
	}
}

void syscall_init()
{
	interrupt_register(0x80, &syscall_handler);
	int test  = ({ int dog = 9; dog;});
	printf("TESET %i\n",test);
}


