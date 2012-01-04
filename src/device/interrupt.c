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
static enum intr_status interrupt_status;

extern void syscall_isr();
//extern void idt_flush(uint32_t);
static void idt_build_entry(idt_entry_t *entry, uint32_t func, uint16_t sel, uint8_t flags);

void pic_init();
void pic_unmask(int irq);
void pic_send_end(int irq);

static void void_handler(struct registers *regs)
{
	if(regs->int_no < NUM_ISRS)
		printf("unhandled interrupt %i\n", regs->int_no);
	else if(regs->int_no - NUM_ISRS != 7)//ignore bochs spurrious interrupt
		printf("unhandled irq %i\n",regs->int_no - NUM_ISRS);
}

void idt_init()
{
	idt_ptr.base = (uint32_t)&idt_table;
	idt_ptr.limit = sizeof(idt_entry_t)*256 - 1;
	kmemset((uint8_t *)idt_table, 0, sizeof(idt_entry_t) * NUM_INTRS);
}


void interrupt_init()
{
	interrupt_disable();
	
	idt_init();
	pic_init();

	for(int i = 0; i < NUM_ISRS + NUM_IRQS; i++)
		idt_build_entry(&idt_table[i], (uint32_t)isrs[i], 0x08, IDT_FLAG_BASE | IDT_FLAG_PRESENT);
	
	for(int i = 0; i < NUM_INTRS; i++)
		interrupt_register(i, void_handler);

	/* needs to be in a seperate function or something */
	extern void sysc();	
	idt_build_entry(&idt_table[0x80], (uint32_t)sysc, 0x08, IDT_FLAG_BASE | IDT_FLAG_PRESENT);

	asm volatile ("lidt (%0)" :: "r"((uintptr_t)&idt_ptr) );
}

void interrupt_register(int irq, intr_handler *handler)
{
	intr_handlers[irq] = handler;
	
	if(irq > NUM_ISRS && irq < NUM_ISRS + NUM_IRQS)
		pic_unmask(irq-20);
}

void interrupt_handler(struct registers *regs)
{
	intr_handler *handler = intr_handlers[regs->int_no];
	
	if(regs->int_no >= NUM_ISRS)
		pic_send_end(regs->int_no - NUM_ISRS);

	if(handler){
		handler(regs); 
	} else {
		printf("something wrong in interrupt.c\n");
	}
}

void dump_regs(struct registers *regs)
{
	printf("edi %X esi %X ebp %X esp %X\nebx %X edx %X ecx %X eax %X\n",
		regs->edi,regs->esi,regs->ebp,regs->esp,regs->ebx,regs->edx,regs->ecx,regs->eax);
	printf("ds %X es %X fs %X gs %X int_no %i err_code %i\n",regs->ds,regs->es,regs->fs,regs->gs,regs->int_no,regs->err_code);
	printf("eip %X cs %X eflags %X useresp %X ss %X\n",
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
	outb(PIC1_DATA, 0x0);
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
	entry->flags = flags|0x60;//needs to be ORed with 0x60 once we get to user mode
	entry->always0 = 0;
	entry->base_lo = ((uint32_t)func) & 0xFFFF;
	entry->base_hi = (((uint32_t)func) >> 16) & 0xFFFF;
} 

enum intr_status interrupt_get()
{
	return interrupt_status;
}

enum intr_status interrupt_disable()
{
	enum intr_status old = interrupt_get();
	asm volatile ("cli"); 
	interrupt_status = INTR_DISABLED;
	
	return old;
}

enum intr_status interrupt_enable()
{
	enum intr_status old = interrupt_get();
	asm volatile ("sti"); 
	interrupt_status = INTR_ENABLED;
	
	return old;
}

enum intr_status interrupt_set(enum intr_status status)
{
	if(status == INTR_ENABLED)
		return interrupt_enable();
	else
		return interrupt_disable();
}


