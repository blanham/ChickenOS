#include <kernel/console.h>
#include <kernel/common.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/fs/vfs.h>
#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

void serial_interrupt(registers_t *regs UNUSED)
{
	console_puts("serial\n");

}
/* serial port major number in Linux is 4 */
void serial_init()
{
//	interrupt_register(IRQ4, serial_interrupt);


}
