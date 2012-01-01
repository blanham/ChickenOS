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
extern int dummy();
//int sh_main(int argc, char **argv)
int sh_main(void *aux)
{
	printf("ChickenOS Shell %s\n",aux);
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
			tmp = dummy();
			printf("returned %x\n",tmp);
			continue;
		}	
		if(strcmp(cmd,"shutdown") == 0)
			shutdown();

		printf("Invalid command '%s'\n",cmd);


	}
}

