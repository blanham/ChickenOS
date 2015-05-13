#ifndef C_OS_ARCH_I386_IDT_H
#define C_OS_ARCH_I386_IDT_H

extern void 
isr0 (), isr1 (), isr2 (), isr3 (), 
isr4 (), isr5 (), isr6 (), isr7 (), 
isr8 (), isr9 (), isr10(), isr11(), 
isr12(), isr13(), isr14(), isr15(),
isr16(), isr17(), isr18(), isr19(),
isr20(), isr21(), isr22(), isr23(),
isr24(), isr25(), isr26(), isr27(),
isr28(), isr29(), isr30(), isr31(),
irq0 (), irq1 (), irq2 (), irq3 (),
irq4 (), irq5 (), irq6 (), irq7 (),
irq8 (), irq9 (), irq10(), irq11(),
irq12(), irq13(), irq14(), irq15();

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
   uint16_t limit : 16;
   uint32_t base : 32;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

void (*isrs[])(void) = {
isr0,  isr1, isr2, isr3, isr4, 
isr5,  isr6, isr7, isr8, isr9, 
isr10, isr11, isr12, isr13, isr14, 
isr15, isr16, isr17, isr18, isr19, 
isr20, isr21, isr22, isr23, isr24, 
isr25, isr26, isr27, isr28, isr29, 
isr30, isr31,
irq0 , irq1 , irq2 , irq3 ,
irq4 , irq5 , irq6 , irq7 ,
irq8 , irq9 , irq10, irq11,
irq12, irq13, irq14, irq15
};

#endif
