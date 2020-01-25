#include <stdio.h>
#include <termios.h>
#include <chicken/common.h>
#include <chicken/device/tty.h>
#include <chicken/device/video/vga.h>
#include <chicken/mm/liballoc.h>

char *termios_cc[NCCS] = {
	"VINTR   ",
	"VQUIT   ",
	"VERASE  ",
	"VKILL   ",
	"VEOF    ",
	"VTIME   ",
	"VMIN    ",
	"VSWTC   ",
	"VSTART  ",
	"VSTOP   ",
	"VSUSP   ",
	"VEOL    ",
	"VREPRINT",
	"VDISCARD",
	"VWERASE ",
	"VLNEXT  ",
	"VEOL2   "
};

char termios[] = {
	0x0,0x2D,0x0,0x0,0x5,0x0,0x0,0x0,0xBF,0x0,0x0,0x0,0x1B,0xCA,0x0,0x0,
	0x0,0x3,0x1C,0x7F,0x15,0x4,0x0,0x1,0x0,0x11,0x13,0x1A,0xFF,0x12,0xF,0x17,
	0x16,0xFF,0x0,0x0,0x1,0x0,0x0,0x0,0xC4,0x12,0xD3,0xBF,0xCC,0x12,0xD3,0xBF,
	0x18,0x12,0xD3,0xBF,0xF5,0x47,0x65,0xB7,0x90,0xD5,0x7A,0xB7
};

/*struct termios
{
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_line;
	cc_t c_cc[NCCS];
	speed_t __c_ispeed;
	speed_t __c_ospeed;
};*/

void tty_termios_print(struct termios *termios)
{
	serial_printf("Iflag %o Oflag %o Cflag %o Lflag %o\n",
			termios->c_iflag, termios->c_oflag, termios->c_cflag, termios->c_lflag);
	serial_printf("Line %x\n", termios->c_line);
	for(int i = 0; i < 16; i++)
	{
		serial_printf("%s = %x ", termios_cc[i], termios->c_cc[i]);
		if(i == 6 || i == 13)
			serial_printf("\n");
	}
	serial_printf("\nIspeed %o Ospeed %o\n", termios->__c_ispeed, termios->__c_ospeed);

}