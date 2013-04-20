#include <device/console.h>
#include <kernel/common.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>
#include <device/serial.h>
#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define RX_BUF 0
#define TX_BUF 0
#define DLLB 0
#define IER  1
#define DLHB 1
#define IIR 2
#define FCR 2
#define LCR 3
#define MCR 4
#define LSR 5
#define MSR 6
#define SCRATCH 7
#define DLAB 0x80

void serial_interrupt(registers_t *regs UNUSED)
{
//	console_puts("serial\n");

}

void serial_putc(char c)
{
	if(c == '\n')
	{
		serial_putc('\r');
	}
	outb(COM1, c);	
}

void serial_set_baud(uint16_t port, uint16_t baud)
{
	outb(port + LCR, DLAB);
	outb(port + DLLB, baud & 0xff);
	outb(port + DLHB, baud >> 8);
}

/* serial port major number in Linux is 4 */
void serial_init()
{
	serial_set_baud(COM1, 0x3);
	outb(COM1 + LCR, 0x0c);
//	interrupt_register(IRQ4, serial_interrupt);
}
