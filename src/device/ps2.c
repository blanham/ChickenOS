#include <common.h>
#include <stdio.h>
#include <chicken/thread.h>
#include <kernel/hw.h>
#include <kernel/memory.h>
#include <device/console.h>
#include <device/input.h>

#define L_CTRL  0x0001
#define R_CTRL  0x0002
#define L_SHIFT 0x0004
#define R_SHIFT 0x0008
#define L_ALT	0x0010
#define R_ALT	0x0020
#define BKSPACE 0x8
#define TAB 0x9
#define ESC 0x1B
#define CR 0xa

uint16_t shifts = 0;

#define BUF_SIZE 256
char rbuf[BUF_SIZE];
uint8_t pos = 0;
uint8_t gpos = 0;

char kbd_map_unshifted[256] = {
	' ',ESC,'1','2','3','4','5','6','7','8','9','0','-','=',BKSPACE,TAB,
	'q','w','e','r','t','y','u','i','o','p','[',']',CR, ' ','a','s',
	'd','f','g','h','j','k','l',';','\'',' ',' ', '\\', 'z','x','c',
	'v','b','n','m',',','.','/', ' ', ' ', ' ', ' '
};

char kbd_map_shifted[256] = {
	' ',ESC,'!','@','#','$','%','^','&','*','(',41,'_','+',BKSPACE,TAB,
	'Q','W','E','R','T','Y','U','I','O','P','{','}',CR, ' ','A','S',
	'D','F','G','H','J','K','L',':','"',' ',' ', '|', 'Z','X','C',
	'V','B','N','M','<','>','?'
};

char kbd_getc()
{
	while(pos == 0);
	return rbuf[--pos];
}

void kbd_e5(uint8_t c)
{
	switch(c)
	{
		case 0x53:
			if((shifts & (L_SHIFT | R_SHIFT)) != 0 && (shifts & (L_CTRL | R_CTRL)) != 0)
				shutdown();

		case 0x2A:
			shifts |= R_CTRL;
			break;
		case 0xAA:
			shifts &= ~R_CTRL;
			break;
		case 0x38:
			shifts |= R_ALT;
			break;
		case 0xB8:
			shifts &= ~R_ALT;
			break;
	}
}

extern uint8_t kbd_read();
extern void reboot();

void ps2_intr(void)
{
	uint8_t c = kbd_read();
	//printf("%x\n", c);
	switch(c)
	{
		case 0x2A:
			shifts |= L_SHIFT;
			break;
		case 0xAA:
			shifts &= ~L_SHIFT;
			break;
		case 0x36:
			shifts |= R_SHIFT;
			break;
		case 0xB6:
			shifts &= ~R_SHIFT;
			break;
		case 0x38:
			shifts |= L_ALT;
			break;
		case 0xB8:
			shifts &= ~L_ALT;
			break;
		case 0x1D:
			shifts |= L_CTRL;
			break;
		case 0x9D:
			shifts &= ~L_CTRL;
			break;
		case 0x3B ... 0x3D:
			//if((shifts & (L_SHIFT | R_SHIFT)) && (shifts & (L_CTRL | R_CTRL)))
			//	console_switch(c - 0x3B);
			break;
	//	case 0x01:
	//		rbuf[pos++] = 0x1B;
	//		break;
		case 15:
			sys_kill(2, SIGCHLD);
			break;

		case 0xe0:
			c = kbd_read();
			if(c & 0x80){
				return;
			}else{
				kbd_e5(c);
				return;
			}
			break;
		/* ESC shutsdown for now */
	//	case ESC:
		//	printf("esc\n");
		//	reboot();
		//	shutdown();
		//	break;
		default:
			if(c & 0x80)
				return;
			if(shifts & (L_SHIFT|R_SHIFT))
				c = kbd_map_shifted[c];
			else
				c = kbd_map_unshifted[c];
					//console_putc(c);
			//TODO: This needs to be in the terminal driver
			if((c == 'c') && (shifts & (L_CTRL)))
			{
				sys_kill(2, SIGINT);
				return;
			}
			if(c == '\\')
			{
				shutdown();
			}
			rbuf[pos++] = c;
		//	input_queue_putc(c);
	}
}

