#include <kernel/console.h>
#include <stdio.h>
#define PANIC(x) panic(__FILE__, __LINE__, x);
static inline void panic(char *file, int line,char* msg)
{
	console_set_color(RED,WHITE);
	console_puts("KERNEL PANIC\n");
	printf("file:%s line:%i [%s]\n",file, line, msg);
	while(1);
}

