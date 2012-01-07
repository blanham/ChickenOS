//#include "user.h"
#include "stdio.h"
enum { SYS_FORK,SYS_GETPID, SYS_EXECV, SYS_PUTS, SYS_DUMMY = 256};

#define SYSCALL_0(num) ({				\
					int ret;			\
	asm volatile ( 	"pushl %1\n"		\
					"int $0x80\n"		\
					"addl $4, %%esp\n"	\
				   	: "=a"(ret) 		\
				   	: "i" (num)			\
					: "memory"			\
				   	);					\
					ret;})


#define SYSCALL_1(num, arg0) ({			\
					int ret;			\
	asm volatile ( 						\
					"pushl %2\n"		\
					"pushl %1\n"		\
					"int $0x80\n"		\
				    "mov %%eax,%0\n"	\
					"addl $8, %%esp\n"	\
				   	: "=a"(ret) 		\
				   	: "i" (num),			\
					 "m" (arg0)		\
					: "memory"			\
				   	);					\
					ret;})
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
int fork()
{
	int test = SYSCALL_0(SYS_FORK);
	return test;
}

/*int uputs(char *str)
{
	int test = SYSCALL_1(SYS_PUTS,str);
	return test;
}*/

int main(int argc, char **argv)
{
//	uputs("hello, world\n");
	printf("hello,world\n");
	printf("argc %x argv %X:%s\n",argc, argv,argv[0]);
	for(int i = 0; i < argc; i++)
		printf("%s\n",argv[i]);
//while(*argv++ != 0)
//	printf("%s\n",*argv);
	
/*	if(fork())
	{
		printf("forked!\n");
		while(1);
	}
	printf("main!\n");	*/
	return 0;
}

void _start(int argc, char **argv)
{

	main(argc, argv);
	while(1);
}
