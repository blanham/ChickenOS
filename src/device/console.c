#include <kernel/common.h>
#include <kernel/memory.h>
#include <device/console.h>
#include <interrupt.h>
#include <kernel/hw.h>
#include <device/tty.h>
#include <device/input.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <device/video/bochs_vga.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>



extern void tty_putc(int c);

void console_putc(int c)
{
	tty_putc(c);
}

//#define CURRENT_CONSOLE consoles[current_console]


//console_t *consoles[5];
//int console_cnt = 0;
//char *termios_cc[NCCS] =
//{
//	"VINTR   ", 
//	"VQUIT   ", 
//	"VERASE  ", 
//	"VKILL   ", 
//	"VEOF    ", 
//	"VTIME   ", 
//	"VMIN    ", 
//	"VSWTC   ", 
//	"VSTART  ", 
//	"VSTOP   ", 
//	"VSUSP   ", 
//	"VEOL    ", 
//	"VREPRINT", 
//	"VDISCARD", 
//	"VWERASE ", 
//	"VLNEXT  ", 
//	"VEOL2   "
//};
//
//
//char console_getc()
//{
//	return kbd_getc();
//}
//void console_putc(uint8_t c)
//{
////	if(consoles[0] != NULL)
//		consoles[0]->putc(consoles[0], c);
////	tty_putc(console, c);
//		
//	serial_putc(c);
//}
//void putchar(uint8_t c)
//{
//	if(consoles[0] != NULL)
//		consoles[0]->putc(consoles[0], c);
////	tty_putc(console, c);
//		
//	serial_putc(c);
//}
//int serial_puts(char *string)
//{
//	int cnt = 0;
//	while(*string != '\0')
//	{
//		serial_putc(*string++);
//		cnt++;
//	}
//
//	return cnt;
//
//
//}
//
//int console_puts(char *string)
//{
//	int cnt = 0;
//	while(*string != '\0')
//	{
//		console_putc(*string++);
//		cnt++;
//	}
//
//	return cnt;
//}
//
//char tty_getc(console_t *con UNUSED)
//{
////	while(console != con);
//			
//	return input_queue_getc();
//}
//void tty_putc(console_t *con, int c)
//{
////	while(console != con);
//			
//	con->putc(con, c);
//	serial_putc(c);
//}
//
//void console_switch(int num)
//{
//	(void)num;
////	consoles[num]->switch_fn(consoles[num]);
//}
//int echo = 1;
//size_t console_read(uint16_t dev UNUSED, void *_buf, off_t off UNUSED, size_t count)
//{
//	char *buf = _buf;
//	size_t ret = 0;
////	size_t read = 0;
//	char c;
//	int tty = MINOR(dev);
//	(void)tty;
//	while(count--)
//	{
//		c = console_getc();//ty_getc(consoles[tty]);
///*		if(c == 0x8)
//		{
//			buf[ret] = '\0';
//			if((signed)ret > 0)
//			{
//				ret--;
//				if(echo)
//					tty_putc(consoles[tty],c);
//
//			}
//		}
//		else
//		{*/
//			if(echo)
//				tty_putc(consoles[tty],c);
//
//			buf[ret] = c;
//			ret++;
//	//	}
//		if(c == 0xa)
//			break;
//	}
//	return ret;//(buf - (char *)_buf);
//}
//
//size_t console_write(uint16_t dev, void *_buf, off_t off UNUSED, size_t count)
//{
//	char *buf = _buf;
//	size_t written = count;
//	int tty = MINOR(dev);
//	(void)buf;
//	while(count--)
//	{
//		
//		tty_putc(consoles[tty],*buf++);
//	}
//
//	return written;
//}
//
//char termios[] = {0x0,0x2D,0x0,0x0,0x5,0x0,0x0,0x0,0xBF,0x0,0x0,0x0,0x1B,0xCA,0x0,0x0,0x0,0x3,0x1C,0x7F,0x15,0x4,0x0,0x1,0x0,0x11,0x13,0x1A,0xFF,0x12,0xF,0x17,0x16,0xFF,0x0,0x0,0x1,0x0,0x0,0x0,0xC4,0x12,0xD3,0xBF,0xCC,0x12,0xD3,0xBF,0x18,0x12,0xD3,0xBF,0xF5,0x47,0x65,0xB7,0x90,0xD5,0x7A,0xB7};
///*struct termios
//{
//	tcflag_t c_iflag;
//	tcflag_t c_oflag;
//	tcflag_t c_cflag;
//	tcflag_t c_lflag;
//	cc_t c_line;
//	cc_t c_cc[NCCS];
//	speed_t __c_ispeed;
//	speed_t __c_ospeed;
//};*/
//void console_termios_print(struct termios *termios)
//{
//	printf("Iflag %x Oflag %x Cflag %x Lflag %x\n", 
//			termios->c_iflag, termios->c_oflag, termios->c_cflag, termios->c_lflag);
//	printf("Line %x\n", termios->c_line);
//	for(int i = 0; i < 16; i++)
//	{
//		printf("%s = %x ", termios_cc[i], termios->c_cc[i]);
//		if(i == 6 || i == 15)
//			printf("\n");
//	}
//	printf("Ispeed %x Ospeed %x\n", termios->__c_ispeed, termios->__c_ospeed);
//
//
//
//}
///*
// *struct winsize {
//	unsigned short ws_row;
//	unsigned short ws_col;
//	unsigned short ws_xpixel;
//	unsigned short ws_ypixel;
//};
//
// *
// * */
////TODO: some of these will probably be taken care of here
////		and some will need to be taken care of in the drivers
//int console_ioctl(uint16_t dev, int request, char *args )
//{
//	int tty = MINOR(dev);
//	static int pgrp = 0;
//	switch(request)
//	{
//		case TCGETS:
//			if(args != 0)
//			{
//				kmemcpy(args, termios, sizeof(termios));
//				//console_termios_print((struct termios *)termios);
//				return 0;
//			}
//			return -EFAULT;
//		case TCSETS:
//			console_termios_print((struct termios *)args);
//				kmemcpy(termios, args, sizeof(termios));
//			printf("SET termios\n");
//			return 0;
//		case TIOCGWINSZ:
//			if(args != 0)
//			{
//				struct winsize wsz;
//				wsz.ws_row = 25;
//				wsz.ws_col = 80;
//				kmemcpy(args, &wsz, sizeof(wsz));
//				return 0;
//			}
//			return -EFAULT;
//		case TIOCGPGRP:
//			*(int *)args = pgrp;
//			return 0;
//		case TIOCSPGRP:
//			pgrp = *(int *)args;
//			return 0;
//
//		default:
//		printf("request %x @ tty %i\n",request,tty);
//			;
//	}
//	return -1;
//}
//
//void console_fs_init()
//{
//	device_register(FILE_CHAR, 0x800, console_read, console_write, console_ioctl);
//	//FIXME: needs to be:
////	device_file_register(0x800, FILE_CHAR, console_ops);
//}
//
//void console_register(console_t *console)
//{
//	console->num = console_cnt;
//	consoles[console_cnt++] = console;
//
//}
