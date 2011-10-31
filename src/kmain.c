#include <kernel/types.h>
#include <stdio.h>
#include <kernel/console.h>
#include <multiboot.h>
#define NULL 0

void panic(char *msg)
{
	console_puts("PANIC:");
	console_puts(msg);
	console_puts("\n");

	while(1);
}
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
void kmain(uint32_t mbd, uint32_t magic)
{
	struct multiboot_info *mb;
   	if ( magic != 0x2BADB002 )
   	{
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
		puts("Bad magic number, halting\r");
		return;
   	}
 	mb = (struct multiboot_info *)mbd;
	
	/* begin initializations */	
	console_init();



	if (CHECK_FLAG (mb->flags, 2))
    	printf ("cmdline = %s\n", (char *) mb->cmdline);
	
	for(int i = 0; i < 43; i++)
	{
		printf("hello, world %b\n",i);
		printf("hello, world %i\n",i);
		printf("hello, world %x\n",i);
	}
	panic("kmain returned");
}
