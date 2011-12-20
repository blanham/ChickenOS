#include <kernel/interrupt.h>
#include <kernel/console.h>
#include <kernel/thread.h>
#include <kernel/syscall.h>
#include <stdio.h>
//#include <kernel/vfs.h>
int return_value = 0;

void syscall_handler (struct registers *regs)
{
	printf("derp %x\n", regs->eax);
	uint32_t *num = (uint32_t*)((uint32_t)regs->useresp);
//	uint32_t res = 0;//*(num);
	
	printf("Res %x\n",regs->useresp);
	printf("Res %x\n",num);
	regs = regs;
	printf("ESP %X\n",regs->esp);	
	int call = (int)((int *)regs->useresp);
	printf("Call %i\n",call);
	regs->eax = 0xfeed;
}


int syscall_haiindler (int num)
{
	num = num;
	printf("derp %X\n",num);	
	return 0xcafebabe;
}

void sys_dummy()
{
	int test = 0;
	int in = 0xcafebabe;
	asm volatile ( "mov %1, %%eax\n"
					"int $0x80\n"
				   "mov %%eax,%0\n"
				   : "=m"(test) 
				   : "m" (in)
				   );
	printf("test %x\n",test);

}
void syscall_init()
{
	interrupt_register(0x80, &syscall_handler);

}

