/*	ChickenOS - interrupt.c
 *  Borrows from Bran and JamesM's tutorials and
 *  the pintos teaching os
 */

#include <kernel/interrupt.h>
#include <stdio.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <kernel/gdt.h>
#include <kernel/hw.h>
#include "i386-defs.h"

#define NUM_IRQS 16
#define NUM_ISRS 32

idt_entry_t idt_table[NUM_INTRS];
idt_ptr_t	idt_ptr;
intr_handler *intr_handlers[NUM_INTRS];


extern void idt_flush(uint32_t);
static void idt_build_entry(idt_entry_t *entry, uint32_t func, uint16_t sel, uint8_t flags);

void pic_init();

void pic_unmask(int irq);
void pic_send_end(int irq);

void void_handler(struct registers *regs)
{
	if(regs->int_no < NUM_ISRS)
		printf("unhandled interrupt %i\n", regs->int_no);
	else
		printf("unhandled irq %i\n",regs->int_no - NUM_ISRS);
}

void interrupt_init()
{
	
	idt_ptr.base = (uint32_t)&idt_table;
	idt_ptr.limit = sizeof(idt_entry_t)*256 - 1;
	
	kmemset((uint8_t *)idt_table, 0, sizeof(idt_entry_t) * NUM_INTRS);

	pic_init();

	for(int i = 0; i < NUM_ISRS + NUM_IRQS; i++)
		idt_build_entry(&idt_table[i], (uint32_t)isrs[i], 0x08, IDT_FLAG_BASE | IDT_FLAG_PRESENT);
	for(int i = 0; i < NUM_INTRS; i++)
		interrupt_register(i, void_handler);
	
	idt_flush((uint32_t)&idt_ptr);

	asm volatile("sti");
}

void interrupt_register(int irq, intr_handler *handler)
{
	intr_handlers[irq] = handler;
	if(irq > NUM_ISRS)
		pic_unmask(1);
}

void interrupt_handler(struct registers *regs)
{
	intr_handler *handler = intr_handlers[regs->int_no];
	if(handler){
		handler(regs); 
	} else {
		printf("something wrong in interrupt.c\n");
	}
	if(regs->int_no > NUM_ISRS)
		pic_send_end(regs->int_no - NUM_ISRS);
}
void dump_regs(struct registers *regs)
{
	printf("edi %x esi %x ebp %x esp %x ebx %x edx %x ecx %x eax %x\n",
		regs->edi,regs->esi,regs->ebp,regs->esp,regs->ebx,regs->edx,regs->ecx,regs->eax);
	printf("eip %x cs %x eflags %x useresp %x ss %x\n",
		regs->eip, regs->cs, regs->eflags, regs->useresp, regs->ss);



}

void pic_init()
{
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
	/* finish init */
	outb(PIC1_DATA, 0x01);//disables system timer for now
	outb(PIC2_DATA, 0x0);
}
void pic_send_end(int irq)
{
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
static void idt_build_entry(idt_entry_t *entry, uint32_t func, uint16_t sel, uint8_t flags)
{
	entry->sel = sel;
	entry->flags = flags;//needs to be ORed with 0x60 once we get to user mode
	entry->always0 = 0;
	entry->base_lo = ((uint32_t)func) & 0xFFFF;
	entry->base_hi = (((uint32_t)func) >> 16) & 0xFFFF;
} 


void test()
{
const char s[] = "Shutdown";
	const char *p;
for (p = s; *p != '\0'; p++)
    outb (0x8900, *p);
	asm volatile ("hlt");



}



