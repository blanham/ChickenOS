#include <common.h>
#include <hw.h>
#include <arch/i386/pic.h>
#include <arch/i386/interrupt.h>

void pic_init()
{
	/* This masks all interrupts */
	outb(PIC1_DATA, 0xff);
	outb(PIC2_DATA, 0xff);

	/* starts init */
	outb(PIC1_CMD, 0x11);
 	outb(PIC2_CMD, 0x11);

	/* set offsets */
	outb(PIC1_DATA, 0x20);
	outb(PIC2_DATA, 0x28);

	/* more init */
	outb(PIC1_DATA, 0x04);
	outb(PIC2_DATA, 0x02);

	/* set 8086 mode */
	outb(PIC1_DATA, 0x01);
	outb(PIC2_DATA, 0x01);

	/* This apparently unmasks all interrupts */
//	outb(PIC1_DATA, 0x0);
//	outb(PIC2_DATA, 0x0);

	/* mask all interrupts */
	for(int i = 0; i < NUM_IRQS; i++)
		pic_mask(i);

	pic_send_end(0);//int irq)
}

void pic_send_end(int irq)
{
	//FIXME: Might want to turn off interrupts here
	if( irq >= 8)
		outb(PIC2_CMD, PIC_EOI);

	outb(PIC1_CMD, PIC_EOI);
}

void pic_mask(int irq)
{
	uint8_t val;
	uint16_t port;

	if(irq < 8)
		port = PIC1_DATA;
	else{
		port = PIC2_DATA;
		irq -= 8;
	}

	val = inb(port) | (1 << irq);
	outb(port, val);
}

void pic_unmask(int irq)
{
	uint8_t val;
	uint16_t port;
	
	if(irq < 8)
		port = PIC1_DATA;
	else{
		port = PIC2_DATA;
		irq -= 8;
	}

	val = inb(port) & ~(1 << irq);
	outb(port, val);
}
