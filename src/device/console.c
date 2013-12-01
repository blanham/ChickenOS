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
#include <sys/ioctls.h>
#define CURRENT_CONSOLE consoles[current_console]

console_t *consoles[5];
int console_cnt = 0;


char console_getc()
{
	return kbd_getc();
}
void console_putc(uint8_t c)
{
	if(consoles[0] != NULL)
		consoles[0]->putc(consoles[0], c);
//	tty_putc(console, c);
		
	serial_putc(c);
}

int console_puts(char *string)
{
	int cnt = 0;
	while(*string != '\0')
	{
		console_putc(*string++);
		cnt++;
	}

	return cnt;
}

char tty_getc(console_t *con UNUSED)
{
//	while(console != con);
			
	return input_queue_getc();
}
void tty_putc(console_t *con, int c)
{
//	while(console != con);
			
	con->putc(con, c);
	serial_putc(c);
}

void console_switch(int num)
{
	consoles[num]->switch_fn(consoles[num]);
}
int echo = 1;
size_t console_read(uint16_t dev UNUSED, void *_buf, off_t off UNUSED, size_t count)
{
	char *buf = _buf;
//	size_t read = 0;
	char c;
	int tty = MINOR(dev);
	(void)tty;
	while(count--)
	{
		c = console_getc();//ty_getc(consoles[tty]);
		if(echo)
		tty_putc(consoles[tty],c);
		*buf = c;
		buf++;
	//	if(c == 0xa)
		//	break;
	}
	return (buf - (char *)_buf);
}

size_t console_write(uint16_t dev, void *_buf, off_t off UNUSED, size_t count)
{
	char *buf = _buf;
	size_t written = count;
	int tty = MINOR(dev);
	(void)buf;
	while(count--)
	{
		
		tty_putc(consoles[tty],*buf++);
	}

	return written;
}

char termios[] = {0x0,0x2D,0x0,0x0,0x5,0x0,0x0,0x0,0xBF,0x0,0x0,0x0,0x1B,0xCA,0x0,0x0,0x0,0x3,0x1C,0x7F,0x15,0x4,0x0,0x1,0x0,0x11,0x13,0x1A,0xFF,0x12,0xF,0x17,0x16,0xFF,0x0,0x0,0x1,0x0,0x0,0x0,0xC4,0x12,0xD3,0xBF,0xCC,0x12,0xD3,0xBF,0x18,0x12,0xD3,0xBF,0xF5,0x47,0x65,0xB7,0x90,0xD5,0x7A,0xB7};
//TODO: some of these will probably be taken care of here
//		and some will need to be taken care of in the drivers
int console_ioctl(uint16_t dev, int request, va_list args )
{
	int tty = MINOR(dev);
	char *ass;
	switch(request)
	{
		case 0x5401://TCGETS
	//	va_start(args);
		ass = va_arg(args, void *);
	//	va_end(args);
		printf("ass %p\n",ass);
		//echo = 0;
		if(ass != 0)
				kmemcpy(ass, termios, sizeof(termios));
			return 0;
		case 0x540f:
			return -1;//12;
		default:
		printf("request %x @ tty %i\n",request,tty);
			;
	}
	return -1;
}

void console_fs_init()
{
	device_register(FILE_CHAR, 0x800, console_read, console_write, console_ioctl);
	//FIXME: needs to be:
//	device_file_register(0x800, FILE_CHAR, console_ops);
}

void console_register(console_t *console)
{
	console->num = console_cnt;
	consoles[console_cnt++] = console;

}
