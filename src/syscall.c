#include <kernel/interrupt.h>
#include <kernel/console.h>
#include "syscall.h"
#include <stdio.h>
//#include <kernel/vfs.h>
int return_value = 0;
int syscall_handler (int);
void syscall_init()
{
	//interrupt_register(0x80, &syscall_handler);

}
int syscall_handler (int test)
{
	printf("hadnerl %x\n", test);
	console_puts("syscall!\n");
	return 0xdeadbeef;
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
