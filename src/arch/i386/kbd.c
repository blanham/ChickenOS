#include <common.h>
#include <device/console.h>
#include <kernel/interrupt.h>
#include <kernel/hw.h>
#include <stdint.h>

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
	return inb(0x60);
}

void kbd_intr(struct registers * regs UNUSED)
{

	extern void ps2_intr(void);
	ps2_intr();
}

void kbd_init()
{
	interrupt_register(0x21, &kbd_intr);
//	ps2_init();
}
