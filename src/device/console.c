#include <kernel/common.h>
#include <kernel/memory.h>
#include <device/console.h>
#include <interrupt.h>
#include <kernel/hw.h>
#include <device/serial.h>
#include <device/input.h>
#include <kernel/vm.h>
#include <kernel/fs/vfs.h>
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
	consoles[0]->putc(consoles[0], c);
//	tty_putc(console, c);
		
//	serial_putc(c);
	(void)c;
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
}

void console_switch(int num)
{
	consoles[num]->switch_fn(consoles[num]);
}

size_t console_read(uint16_t dev UNUSED, void *_buf, off_t off UNUSED, size_t count)
{
	char *buf = _buf;
	size_t read = 0;
	int tty = MINOR(dev);
	(void)tty;
	while(count--)
	{
		read++;
		*buf++ = console_getc();//ty_getc(consoles[tty]);
		tty_putc(consoles[tty],*(buf-1));
		if(*(buf - 1) == '\n')
			break;
	}
//	*buf = '\0';	
	*buf = '\n';
	return read;
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
//TODO: some of these will probably be taken care of here
//		ans some will need to be taken care of in the drivers
int console_ioctl(uint16_t dev, int request, va_list args UNUSED)
{
	int tty = MINOR(dev);
	switch(request)
	{
	//	case 0x5401://TCGETS
		//	return 0;
	//	case 0x540f:
		//	return 12;
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
