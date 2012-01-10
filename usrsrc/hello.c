//#include "user.h"
#include <stdio.h>
#include <stdlib.h>
#include "../src/include/thread/syscall.h"
//enum { SYS_FORK,SYS_GETPID, SYS_EXECV, SYS_PUTS, SYS_DUMMY = 256};

/*
int dummy()
{
	int test = SYSCALL_0(SYS_DUMMY);
	return test;
}
pid_t get_pid()
{
	int test = SYSCALL_0(SYS_GETPID);
	return test;
}*/
/*int uputs(char *str)
{
	int test = SYSCALL_1(SYS_PUTS,str);
	return test;
}*/

int main(int argc, char **argv)
{
	int esp;
	asm volatile("mov %%esp, %0": "=m"(esp));
	printf("hello,world%x\n",esp);
	printf("argc %x argv %X\n",argc, argv);
//	for(int i = 0; i < argc; i++)
	//	printf("%s\n",argv[i]);
	while(1);
	//	printf("derp\n");
	//	printf("%s\n",*argv);
//	exit(0);
//	if(fork())
//	{
//		printf("forked!\n");
//		while(1);
//	}
//	printf("main!\n");	
//	while(1);
	return 0;
}
/*
void _start(int argc, char **argv)
{

	main(argc, argv);
	while(1);
}*/
