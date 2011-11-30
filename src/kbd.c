#include <kernel/console.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
uint16_t shifts;
#define L_CTRL  0x0001
#define R_CTRL  0x0002
#define L_SHIFT 0x0004
#define R_SHIFT 0x0008
#define BKSPACE 0x8
#define TAB 0x9
#define ESC 0x1B
#define CR 0xa

#define BUF_SIZE 256
char rbuf[BUF_SIZE];
uint8_t pos = 0;
uint8_t gpos = 0;


char kbd_map_unshifted[256] = {
' ',ESC,'1','2','3','4','5','6','7','8','9','0','-','+',BKSPACE,TAB,
'q','w','e','r','t','y','u','i','o','p',' ',' ',CR, ' ','a','s',
'd','f','g','h','j','k','l',';','\'',' ',' ', ' ', 'z','x','c',
'v','b','n','m',',','.','/'


};
char kbd_map_shifted[256] = {
' ',ESC,'!','@','#','$','%','^','&','*','(',41,'_','+',BKSPACE,TAB,
'Q','W','E','R','T','Y','U','I','O','P',' ',' ',CR, ' ','A','S',
'D','F','G','H','J','K','L',';','|',' ',' ', ' ', 'Z','X','C',
'V','B','N','M','<','>','?'


};
char kbd_getc()
{
	while(pos == 0);
	return rbuf[--pos];
}
void kbd_intr()//;//struct registers * regs)
{
	uint8_t c = inb(0x60);
	
	switch(c)
	{
		
		case 0x2A:
			shifts |= R_SHIFT;
			break;
		case 0xAA:
			shifts &= ~R_SHIFT;
			break;
		case 0xe0:
			c = inb(0x60);
			if(c & 0x80)
				return;
			break;
		/* ESC shutsdown for now */
		case 1:
			shutdown();
		default:
			if(c & 0x80)
				return;
			if(shifts & R_SHIFT)
				c = kbd_map_shifted[c];
			else
				c = kbd_map_unshifted[c];
			//console_putc(c);
			rbuf[pos++] = c;
	}
}
void kbd_init()
{
	
	interrupt_register(0x21, &kbd_intr);

}

