#include <stdint.h>
#include <chicken/common.h>
#include <chicken/device/console.h>
#include <chicken/device/ioport.h>
#include <chicken/interrupt.h>


typedef struct {
	ioport_t io;
} i8042_t;

i8042_t i8042 = {.io = {.base = 0x60}};

#define I8042_RW_DATA	0x00
#define I8042_R_STATUS	0x04
#define I8042_W_COMMAND	0x04

/* FIXME: Maybe move this somewhere else? */
void reboot()
{
	uint8_t test = 0x02;
	while((test & 0x02) != 0)
		test = inb(0x64);
	outb(0x64, 0xFE);
	asm volatile ("hlt");
}

uint8_t kbd_read()
{
	//return inb(0x60);
	return io_inb(&i8042.io, I8042_RW_DATA);
}

void kbd_intr(registers_t * regs UNUSED)
{
	extern void ps2_intr(void);
	ps2_intr();
}

void kbd_init()
{
	interrupt_register(IRQ1, &kbd_intr);
//	ps2_init();
}