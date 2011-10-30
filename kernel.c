#include "multiboot.h"
#define NULL 0
unsigned char * videoram;
unsigned char * videocur;



void putc(char c)
{
	static int count;
	switch(c)
	{
		case '\r':
			videocur += 160-count*2;
			count = 0;
			// videoram + ((videocur - videoram)/160);
			break;
		default:
		*videocur++ = c;
		*videocur++  = 0x07;
		count++;
	}
	if(count == 81) count = 0;
	if(videocur > (videoram + 160*50))
		videocur = videoram;
}
int prints(char *string)
{
	int cnt = 0;
	while(string[cnt] != NULL)
	{
		putc(string[cnt++]);
	}


	return cnt;
}

void clearscreen()
{
	int i = 0;
	for(i = 0; i < 80*50; i++)
	{
		*videocur++ = 0;
		*videocur++ = 0;
	}

	videocur = (unsigned char *) 0xb8000;

}
void kmain( void* mbd, unsigned int magic )
{
   if ( magic != 0x2BADB002 )
   {
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
   }
	struct multiboot_info_t *mb = (struct multiboot_info_t *)mbd;
	mb = mb;
   /* You could either use multiboot.h */
   /* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
   /* or do your offsets yourself. The following is merely an example. */ 
  // char * boot_loader_name =(char*) ((long*)mbd)[16];
   /* Print a letter to screen to see everything is working: */
//   unsigned char *
videoram = videocur = (unsigned char *) 0xb8000;
//	int i;
	clearscreen();
	unsigned char *test = (unsigned char *)0;
	test = test;
//	int len = 40*24;
	/*for(i = 0; i < len; i++)
	{
		*videoram++ = 65;
		*videoram++ = 0x07;
   	}*/
	int i =0;
	for(i = 0; i < 24; i++)
	prints("hello, world\r");
	//char *boot_loader_name =(char *)(mb->)boot_loader_name; 
	//prints("test\r");
	//videoram[0] = 65; /* character 'A' */
   //videoram[1] = 0x07; /* light grey (7) on black (0). */
}
