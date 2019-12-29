/*	ChickenOS - i386/interrupt.c
 */

#include <common.h>
#include <stdio.h>
#include <chicken/thread.h>
#include <kernel/memory.h>
#include <thread/syscall.h>
#include <arch/i386/pic.h>
#include <arch/i386/interrupt.h>
#include "i386_defs.h"

extern idt_entry_t idt_table[NUM_INTRS];
extern void sysc();

intr_handler *intr_handlers[NUM_INTRS];
idt_ptr_t	idt_ptr;

// FIXME: This is ugly
void (*isrs[])(void) = {
	isr0,  isr1,  isr2,  isr3,  isr4,
	isr5,  isr6,  isr7,  isr8,  isr9,
	isr10, isr11, isr12, isr13, isr14,
	isr15, isr16, isr17, isr18, isr19,
	isr20, isr21, isr22, isr23, isr24,
	isr25, isr26, isr27, isr28, isr29,
	isr30, isr31,
	irq0,  irq1,  irq2,  irq3,
	irq4,  irq5,  irq6,  irq7,
	irq8,  irq9,  irq10, irq11,
	irq12, irq13, irq14, irq15
};

static void void_handler(registers_t *regs)
{
	if(regs->int_no < NUM_ISRS)
		serial_printf("unhandled interrupt %i\n", regs->int_no);
	else if(regs->int_no - NUM_ISRS != 7) // This will ignore bochs spurious interrupt
		serial_printf("unhandled irq %i\n",regs->int_no - NUM_ISRS);
}

static void i386_syscall_handler(registers_t *regs)
{
	//dump_regs(regs);
	thread_current()->registers = regs;
	syscall_handler(regs);
}

static void gpf(registers_t *regs)
{
	// TODO: Still would love to make this work again
	// console_set_color(LT_GREY,WHITE);
	printf("PID: %i\n",thread_current()->pid);
	dump_regs(regs);
	BOCHS_BREAK();
	PANIC("GENERAL PROTECTION FAULT!");
}

static void page_fault(registers_t * regs)
{
	uintptr_t faulting_addr;
	asm volatile ("mov %%cr2, %0":"=r" (faulting_addr));
	vm_page_fault(regs, faulting_addr, regs->err_code & PAGE_ERR_MASK);
}

static void double_fault(registers_t * regs)
{
	dump_regs(regs);
	BOCHS_BREAK();
	PANIC("DOUBLE FAULT!");
}

static void invalid_opcode(registers_t * regs)
{
	dump_regs(regs);
	BOCHS_BREAK();
	PANIC("INVALID OPCODE!");
}

static void divide_error(registers_t * regs)
{
	dump_regs(regs);
	BOCHS_BREAK();
	PANIC("DIVISION BY ZERO!");
}

static void stack_exception(registers_t * regs)
{
	dump_regs(regs);
	BOCHS_BREAK();
	PANIC("STACK EXCEPTION!");
}

static void other_exception(registers_t * regs)
{
	dump_regs(regs);
	BOCHS_BREAK();
	PANIC("UNHANDLED EXCEPTION!");
}

void interrupt_register(int irq, intr_handler *handler)
{
	intr_handlers[irq] = handler;

	if(irq >= NUM_ISRS && irq <= NUM_ISRS + NUM_IRQS)
		pic_unmask(irq - NUM_ISRS);
}

void interrupt_handler(registers_t *regs)
{
	intr_handlers[regs->int_no](regs);

	if(regs->int_no >= NUM_ISRS)
		pic_send_end(regs->int_no - NUM_ISRS);
}

void idt_init()
{
	idt_ptr.base = (uintptr_t)&idt_table;
	idt_ptr.limit = sizeof(idt_entry_t)*256 - 1;
	kmemset(idt_table, 0, sizeof(idt_entry_t) * NUM_INTRS);
}

static void idt_build_entry(idt_entry_t *entry, void *func, uint16_t sel, uint8_t flags)
{
	uintptr_t func_ptr = (uintptr_t)func;
	entry->sel = sel;
	entry->flags = flags | IDT_FLAG_RING3;
	entry->always0 = 0;
	entry->base_lo = func_ptr & 0xFFFF;
	entry->base_hi = (func_ptr >> 16) & 0xFFFF;
}

void arch_interrupt_init()
{
	arch_interrupt_disable();

	idt_init();

	pic_init();

	for(int i = 0; i < NUM_ISRS + NUM_IRQS; i++)
		idt_build_entry(&idt_table[i], isrs[i], 0x08, IDT_FLAG_BASE | IDT_FLAG_PRESENT);

	for(int i = 0; i < NUM_INTRS; i++)
		interrupt_register(i, void_handler);

	idt_build_entry(&idt_table[0x80], sysc, 0x08, IDT_FLAG_BASE | IDT_FLAG_PRESENT);

	asm volatile ("lidt (%0)"::"r"(&idt_ptr));

	// FIXME: Some of these panic when they shouldn't :)
	interrupt_register(0,  &divide_error);		// Divide-by-zero (fault)
	interrupt_register(1,  &other_exception);	// Debug exception (trap or fault)
	interrupt_register(2,  &other_exception);	// Non-Maskable Interrupt (NMI) (trap)
	interrupt_register(3,  &other_exception);	// Breakpoint (INT 3) (trap)
	interrupt_register(4,  &other_exception);	// Overflow (INTO with EFlags[OF] set) (trap)
	interrupt_register(5,  &other_exception);	// Bound exception (BOUND on out-of-bounds access) (trap)
	interrupt_register(6,  &invalid_opcode);	// Invalid Opcode (trap)
	interrupt_register(7,  &other_exception);	// FPU not available (trap)
	interrupt_register(8,  &double_fault);		// Double Fault (abort)
	interrupt_register(9,  &other_exception);	// Coprocessor Segment Overrun (abort)
	interrupt_register(10, &other_exception);	// Invalid TSS (fault)
	interrupt_register(11, &other_exception);	// Segment not present (fault)
	interrupt_register(12, &stack_exception);	// Stack exception (fault)
	interrupt_register(13, &gpf);       		// General Protection (fault or trap)
	interrupt_register(14, &page_fault);		// Page fault (fault)
	interrupt_register(15, &other_exception);	// Reserved
	interrupt_register(16, &other_exception);	// Floating-point error (fault)
	interrupt_register(17, &other_exception);	// Alignment Check (fault)
	interrupt_register(18, &other_exception);	// Machine Check (abort)

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

void dump_regs(registers_t *regs)
{
	serial_printf("PID %i\n", thread_current()->pid);
	serial_printf("edi %X esi %X ebp %X esp %X\nebx %X edx %X ecx %X eax %X\n",
		regs->edi,regs->esi,regs->ebp,regs->esp,regs->ebx,regs->edx,regs->ecx,regs->eax);
	serial_printf("ds %X es %X fs %X gs %X int_no %i err_code %X\n",
		regs->ds,regs->es,regs->fs,regs->gs,regs->int_no,regs->err_code);
	serial_printf("eip %X cs %X eflags %X useresp %X ss %X\n",
		regs->eip, regs->cs, regs->eflags, regs->useresp, regs->ss);
}
