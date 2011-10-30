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

void kmain( void* mbd, uint32_t magic )
{
   	if ( magic != 0x2BADB002 )
   	{
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
		puts("Bad magic number, halting\r");
		return;
   	}
	struct multiboot_info_t *mb = (struct multiboot_info_t *)mbd;
	mb = mb;
   /* You could either use multiboot.h */
   /* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
   /* or do your offsets yourself. The following is merely an example. */ 
   //char * boot_loader_name =(char*) ((long*)mbd)[16];
	console_init();
	for(int i = 0; i < 43; i++)
	{
		printf("hello, world %b\n",i);
		printf("hello, world %i\n",i);
		printf("hello, world %x\n",i);
	}
/*	int i = 0xDEADBEEF;
	int j = 1234567890;
	int k = 42;
	printf("hello, %x %i %b %c %sworld\n",i,j,k,'c',"fuck");
*/
	panic("kmain returned\0");
}
