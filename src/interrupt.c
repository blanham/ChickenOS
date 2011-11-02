#include <kernel/interrupt.h>
#include <stdio.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <kernel/gdt.h>
#include <kernel/hw.h>

#define NUM_INTRS 256
/* IDT flags */
#define IDT_FLAG_BASE 0x0E//always
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0   0x00
#define IDT_FLAG_RING1   0x20
#define IDT_FLAG_RING2   0x40
#define IDT_FLAG_RING3	 0x60
/* structs taken from James Molloy's kernel tutorial */
struct idt_entry_struct
{
   uint16_t base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
   uint16_t sel;                 // Kernel segment selector.
   uint8_t  always0;             // This must always be zero.
   uint8_t  flags;               // More flags. See documentation.
   uint16_t base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
   uint16_t limit;
   uint32_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;
idt_entry_t idt_table[NUM_INTRS];
idt_ptr_t	idt_ptr;

extern void idt_flush(uint32_t);
static void idt_build_entry(idt_entry_t *entry, void *func, uint16_t sel, uint8_t flags);
/*
void (*isrs[])(void) = {
isr0,  isr1, isr2, isr3, isr4, 
isr5,  isr6, isr7, isr8, isr9, 
isr10, isr11, isr12, isr13, isr14, 
isr15, isr16, isr17, isr18, isr19, 
isr20, isr21, isr22, isr23, isr24, 
isr25, isr26, isr27, isr28, isr29, 
isr30, isr31};*/
void test()
{
}
void interrupt_init()
{
	idt_ptr.base = (uint32_t)idt_table;
	idt_ptr.limit = sizeof(idt_entry_t)*256 - 1;
	kmemset((uint8_t *)idt_table, 0, sizeof(idt_entry_t) * NUM_INTRS);
	uint8_t flags = IDT_FLAG_BASE | IDT_FLAG_RING0;
	uint16_t sel = 0x08;
	for(int i = 0; i < NUM_INTRS; i++)
		idt_build_entry(&idt_table[i], test, sel, flags);
//	for(int i = 0; i < 32; i++)
//		idt_build_entry(&idt_table[i], isrs[i], sel, flags);

	idt_flush((uint32_t)&idt_ptr);
}

static void idt_build_entry(idt_entry_t *entry, void *func, uint16_t sel, uint8_t flags)
{
	entry->sel = sel;
	entry->flags = flags;//needs to be ORed with 0x60 once we get to user mode
	entry->base_lo = ((uint32_t)func) & 0xFFFF;
	entry->base_hi = (((uint32_t)func) >> 16) & 0xFFFF;
} 

