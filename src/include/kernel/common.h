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


#include <kernel/console.h>
#include <stdio.h>
#define PANIC(x) panic(__FILE__, __LINE__, x);
static inline void panic(char *file, int line,char* msg)
{
	console_set_color(RED,WHITE);
	console_puts("KERNEL PANIC\n");
	printf("file:%s line:%i [%s]\n",file, line, msg);
	kernel_halt();
}

#endif
