#include <chicken/common.h>
#include <chicken/device/ioport.h>
#include <chicken/device/console.h>
#include <chicken/interrupt.h>
#include "serial.h"

void serial_interrupt(registers_t *regs UNUSED)
{
//	console_puts("serial\n");
}

int serial_received()
{
	return 0;
}

int serial_ready()
{
	return 0;
}

void serial_putc(char c)
{
	//FIXME: should be a line discipline thing probably?
	//NOTE: From mentions online yes it is

	if(c == '\n')
	{
		serial_putc('\r');
	}

	while(serial_ready() == 0);

	//outb(com_port, c);
	//outb(0xE9, c);
}

int serial_getc()
{
	while(serial_received() == 0);

	return 0;//inb(com_port);
}

void serial_set_baud(uint16_t port, uint16_t baud)
{
	outb(port + LCR, DLAB);
	outb(port + DLLB, baud & 0xff);
	outb(port + DLHB, baud >> 8);
	outb(port + LCR, 0x03); // 8N1
}

/* serial port major number in Linux is 4 */
void serial_init()
{
	/*
	int count = 0;
	//probe serial ports from bios data
	uint16_t *com = (uint16_t *)0xC0000400;
	for (int i = 0; i < 4; i++)
	{
		if(com[i] != 0)
		{
			count++;
			//FIXME: log serial probing
			//log/printf("Found COM port @ 0x%X\n", com[i]);
			com_port = com[i];
			break;
		}
	}
	outb(com_port + IER, 0x00); //disable interrupts
	serial_set_baud(com_port, 0x3);
//outb(com_port + FCR, 0xC7); //enable fifo, clear it, 14 bytes
	outb(com_port + FCR, 0x0); //enable fifo, clear it, 14 bytes
	outb(com_port + LCR, 0x03);
	//interrupt_register(IRQ4, serial_interrupt);
	*/
}