#include <common.h>
#include <device/console.h>
#include <kernel/hw.h>
#include <interrupt.h>
#include <arch/i386/serial.h>

uint16_t com_port = COM1;

//FIXME: Just polls for now, should really start as polling
//		 once system is far enough along switch to interrupts
void serial_interrupt(registers_t *regs UNUSED)
{
//	console_puts("serial\n");

}

int serial_received()
{
	return inb(com_port + LSR) & 1;
}

int serial_ready()
{
	return inb(com_port + LSR) & 0x20;
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

	outb(com_port, c);
	outb(0xE9, c);
}

int serial_getc()
{
	while(serial_received() == 0);

	return inb(com_port);
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
}

void pc_serial_init()
{




}

