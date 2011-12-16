#include <stdio.h>
//#include <stdlib.h>
#include <stdio.h>
//#include <vfs.h>
#include <string.h>
#include <kernel/list.h>

struct command {
	int (*cmd)(void *aux);
	char name[32];
	struct list_head list;

};

extern void shutdown();

void sh_gets(char *str)
{
	int i = 0;
	char c;
	while( i < 32)
	{
		c = getchar();
		putc(c);

		if(c == '\n')
			return;
		*str++ = c;
	}
}
int test;
int call(int derp)
{
	test = 0xcafebabe;
	int in = derp;
	uint32_t _esp;
	asm volatile ("mov %%esp, %0"
					:"=m"(_esp)
					);
	printf("ESP %X\n",_esp);

	asm volatile ( "pushl %1\n"
					"int $0x80\n"
				   "mov %%eax,%0\n"
					"addl $4, %%esp\n"
				   : "=m"(test) 
				   : "m" (in)
					: "memory"
				   );
	printf("test %x\n",test);

	return test;
}

//int sh_main(int argc, char **argv)
int sh_main(void *aux)
{
	printf("ChickenOS Shell\n");
	char cmd[32];
	aux = aux;
	int tmp;
	while(1)
	{
		
		printf("$ ");
		memset(cmd,0,32);
		sh_gets(cmd);	
		if(strcmp(cmd,"exit") == 0)
			return 1;
		if(strcmp(cmd, "call") == 0)
		{
			tmp=call(0xdeabeef);
			printf("returned %x\n",tmp);
			continue;
		}	
		if(strcmp(cmd,"shutdown") == 0)
			shutdown();

		printf("Invalid command '%s'\n",cmd);


	}
}

