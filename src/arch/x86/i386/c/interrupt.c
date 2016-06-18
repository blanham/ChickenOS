/*	ChickenOS - i386/interrupt.c
 *  Borrows from Bran and JamesM's tutorials and
 *  the pintos teaching os
 */
#include <common.h>
#include <kernel/interrupt.h>
#include <stdio.h>
#include <kernel/thread.h>
#include <device/console.h>
#include <kernel/memory.h>
#include <kernel/hw.h>
#include <i386_interrupt.h>
#include "pic.h"
#include "idt.h"

//#define DEBUG_INTR

extern idt_entry_t idt_table[NUM_INTRS];
idt_ptr_t	idt_ptr;
intr_handler *intr_handlers[NUM_INTRS];

extern void sysc();

void dump_regs(struct registers *regs)
{
	printf("ESP %X\n", regs->ESP);
	printf("edi %X esi %X ebp %X esp %X\nebx %X edx %X ecx %X eax %X\n",
		regs->edi,regs->esi,regs->ebp,regs->esp,regs->ebx,regs->edx,regs->ecx,regs->eax);
	printf("ds %X es %X fs %X gs %X int_no %i err_code %i\n",
		regs->ds,regs->es,regs->fs,regs->gs,regs->int_no,regs->err_code);
	printf("eip %X cs %X eflags %X useresp %X ss %X\n",
		regs->eip, regs->cs, regs->eflags, regs->useresp, regs->ss);
}

static void void_handler(struct registers *regs)
{
	if(regs->int_no < NUM_ISRS)
		;//printf("unhandled interrupt %i\n", regs->int_no);
	else if(regs->int_no - NUM_ISRS != 7)//ignore bochs spurrious interrupt
		serial_printf("unhandled irq %i\n",regs->int_no - NUM_ISRS);
	while(1);
}

static void gpf(struct registers *regs)
{
	uint32_t error_code = regs->err_code;
	asm volatile("xchg %bx, %bx");
	interrupt_disable();
	//console_set_color(LT_GREY,WHITE);
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	print_user_trace((void *)regs->ebp);
	asm volatile("xchg %bx, %bx");
	PANIC("GENERAL PROTECTION FAULT!");
}

static void double_fault(struct registers * regs)
{
	uint32_t error_code = regs->err_code;
	asm volatile("xchg %bx, %bx");
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	asm volatile("xchg %bx, %bx");
	PANIC("DOUBLE FAULT!");
}

static void opcode_fault(struct registers * regs)
{
	uint32_t error_code = regs->err_code;
	asm volatile("xchg %bx, %bx");
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	asm volatile("xchg %bx, %bx");
	PANIC("INVALID OPCODE!");
}

static void divide_fault(struct registers * regs)
{
	uint32_t error_code = regs->err_code;
	asm volatile("xchg %bx, %bx");
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	asm volatile("xchg %bx, %bx");
	PANIC("DIVIDE FAULT!");
}

static void stack_fault(struct registers * regs)
{
	uint32_t error_code = regs->err_code;
	asm volatile("xchg %bx, %bx");
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	asm volatile("xchg %bx, %bx");
	PANIC("STACK FAULT!");
}

static void page_fault(struct registers * regs)
{
	uintptr_t faulting_addr;

	asm volatile (
		"mov %%cr2, %0"
		: "=r" (faulting_addr)
	);

	vm_page_fault(regs, faulting_addr, regs->err_code &
							(PAGE_USER | PAGE_WRITE | PAGE_VIOLATION));
	//In theory we only return after setting things right
	return;
}

static void other_fault(struct registers * regs)
{
	uint32_t error_code = regs->err_code;
	asm volatile("xchg %bx, %bx");
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	PANIC("OTHER FAULT!");
}

void interrupt_register(int irq, intr_handler *handler)
{
	intr_handlers[irq] = handler;

	if(irq >= NUM_ISRS && irq <= NUM_ISRS + NUM_IRQS)
		pic_unmask(irq-0x20);
}

void interrupt_handler(struct registers *regs)
{
	int irq = regs->int_no;
	intr_handler *handler = intr_handlers[irq];

#ifdef DEBUG_INTR
	if(irq > 32 && irq < 32+32)
		printf("irq %i\n",irq-32);
#endif

	if(handler){
		handler(regs);
	} else {
		printf("something wrong in interrupt.c\n");
	}

	if(regs->int_no >= NUM_ISRS)
		pic_send_end(regs->int_no - NUM_ISRS);
}


void idt_init()
{
	idt_ptr.base = (uint32_t)&idt_table;
	idt_ptr.limit = sizeof(idt_entry_t)*256 - 1;
	kmemset((uint8_t *)idt_table, 0, sizeof(idt_entry_t) * NUM_INTRS);
}

static void idt_build_entry(idt_entry_t *entry, uint32_t func, uint16_t sel, uint8_t flags)
{
	entry->sel = sel;
	entry->flags = flags | 0x60;//needs to be ORed with 0x60 once we get to user mode
	entry->always0 = 0;
	entry->base_lo = ((uint32_t)func) & 0xFFFF;
	entry->base_hi = (((uint32_t)func) >> 16) & 0xFFFF;
}

void arch_interrupt_init()
{
//	interrupt_disable();

	idt_init();

	pic_init();

	for(int i = 0; i < NUM_ISRS + NUM_IRQS; i++)
		idt_build_entry(&idt_table[i], (uint32_t)isrs[i], 0x08, IDT_FLAG_BASE | IDT_FLAG_PRESENT);

	for(int i = 0; i < NUM_INTRS; i++)
		interrupt_register(i, void_handler);

	/* needs to be in a seperate function or something */
	idt_build_entry(&idt_table[0x80], (uint32_t)sysc, 0x08, IDT_FLAG_BASE | IDT_FLAG_PRESENT);

	/* load idt into processor */
	asm volatile ("lidt (%0)" :: "r"((uintptr_t)&idt_ptr) );

	//Some of these will need to send signals eventually
	interrupt_register(0, &divide_fault);	//Divide-by-zero	fault
	interrupt_register(1, &other_fault);	//Debug exception	trap or fault
	interrupt_register(2, &other_fault);	//Non-Maskable Interrupt (NMI)	trap
	interrupt_register(3, &other_fault);	//Breakpoint (INT 3)	trap
	interrupt_register(4, &other_fault);	//Overflow (INTO with EFlags[OF] set)	trap
	interrupt_register(5, &other_fault);	//Bound exception (BOUND on out-of-bounds access)	trap
	interrupt_register(6, &opcode_fault);	//Invalid Opcode	trap
	interrupt_register(7, &other_fault);	//FPU not available	trap
	interrupt_register(8, &double_fault);	//Double Fault	abort
	interrupt_register(9, &other_fault);	//Coprocessor Segment Overrun	abort
	interrupt_register(10, &other_fault);	//Invalid TSS	fault
	interrupt_register(11, &other_fault);	//Segment not present	fault
	interrupt_register(12, &stack_fault);	//Stack exception	fault
	interrupt_register(13, &gpf);       	//General Protection	fault or trap
	interrupt_register(14, &page_fault);	//Page fault	fault
	interrupt_register(15, &other_fault);	//Reserved
	interrupt_register(16, &other_fault);	//Floating-point error	fault
	interrupt_register(17, &other_fault);	//Alignment Check	fault
	interrupt_register(18, &other_fault);	//Machine Check	abort

	interrupt_register(0x80, &i386_syscall_handler);
}

void arch_interrupt_enable()
{
	asm volatile ("sti");
}

void arch_interrupt_disable()
{
	asm volatile ("cli");
}

