/*	ChickenOS - i386/i386_defs.h
 */


// TODO: Major cleanup

#ifndef C_OS_I386_DEFS_H
#define C_OS_I386_DEFS_H

#define BOCHS_BREAK() asm volatile("xchg %bx, %bx")

#define PDE_MASK	0xffc00000
#define PDE_SHIFT 	22
#define PDE_P		0x00000001
#define PDE_RW		0x00000002
#define PDE_USER	0x00000004
#define PDE_WTHRU	0x00000008
#define	PDE_DCACHE	0x00000010
#define PDE_ACCESED	0x00000020
#define PDE_DIRTY	0x00000040
#define PDE_GLOBAL	0x00000080
#define PDE_USER0	0x00000100
#define PDE_USER1	0x00000200
#define PDE_USER2	0x00000400

#define PTE_MASK	0x003ff000
#define PTE_SHIFT 	12
#define PTE_P		PDE_P
#define PTE_RW		PDE_RW
#define PTE_USER	PDE_USER
#define PTE_WTHRU	PDE_WTHRU
#define	PTE_DCACHE	PDE_DCACHE
#define PTE_ACCESED	PDE_ACCESED
#define PTE_DIRTY	PDE_DIRTY
#define PTE_USER0	PDE_USER0
#define PTE_USER1	PDE_USER1
#define PTE_USER2	PDE_USER2

#define PAGEDIR_MASK 0xFFFFE000

#define PDE_COUNT 1024
#define PTE_COUNT 1024
#define PTE_SIZE  4096
#define PDE_SIZE  4096
#define KERNEL_PDE_START 768

struct segment_descriptor {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base16;
	uint8_t access;
	uint8_t flags;
	uint8_t base24;
} __attribute__((packed));
typedef struct segment_descriptor gdt_seg_t;

struct gdt_descriptor {
	uint16_t size;
	uint32_t location;
} __attribute__((packed));

#define GDTA_P       0x80
#define GDTA_RING0   0x00
#define GDTA_RING3   0x60
#define GDTA_ALWAYS  0x10
#define GDTA_EXE     0x08
#define GDTA_CANJUMP 0x04
#define GDTA_RW      0x02
#define GDTA_ACCESS  0x01
#define GDTF_32BIT   0x40
#define GDTF_4KB 	   0x80

#define GDTA_KERNEL        (GDTA_P | GDTA_RING0 | GDTA_ALWAYS | GDTA_EXE  | GDTA_RW)
#define GDTA_KERNEL_DATA   (GDTA_P | GDTA_RING0 | GDTA_ALWAYS | GDTA_RW)
#define GDTA_USER          (GDTA_P | GDTA_RING3 | GDTA_ALWAYS | GDTA_EXE |  GDTA_RW)
#define GDTA_USER_DATA     (GDTA_P | GDTA_RING3 | GDTA_ALWAYS | GDTA_RW)
#define GDTA_TSS           (GDTA_P | GDTA_RING3 | GDTA_EXE | GDTA_ACCESS)
#define GDTF_BOTH          (GDTF_32BIT | GDTF_4KB)

extern void gdt_flush(struct gdt_descriptor *ptr);

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

#define IDT_FLAG_BASE    0x0E
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0   0x00
#define IDT_FLAG_RING1   0x20
#define IDT_FLAG_RING2   0x40
#define IDT_FLAG_RING3	 0x60

typedef struct
{
   uint16_t base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
   uint16_t sel;                 // Kernel segment selector.
   uint8_t  always0;             // This must always be zero.
   uint8_t  flags;               // More flags. See documentation.
   uint16_t base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed)) idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
typedef struct 
{
   uint16_t limit;
   uint32_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed)) idt_ptr_t;

typedef uint32_t sel_t;
typedef struct tss {
    sel_t		oldtss;
    unsigned int		:0;
    unsigned int	esp0;
    sel_t		ss0;
    unsigned int		:0;
    unsigned int	esp1;
    sel_t		ss1;
    unsigned int		:0;
    unsigned int	esp2;
    sel_t		ss2;
    unsigned int		:0;
    unsigned int	cr3;
    unsigned int	eip;
    unsigned int	eflags;
    unsigned int	eax;
    unsigned int	ecx;
    unsigned int	edx;
    unsigned int	ebx;
    unsigned int	esp;
    unsigned int	ebp;
    unsigned int	esi;
    unsigned int	edi;
    sel_t		es;
    unsigned int		:0;
    sel_t		cs;
    unsigned int		:0;
    sel_t		ss;
    unsigned int		:0;
    sel_t		ds;
    unsigned int		:0;
    sel_t		fs;
    unsigned int		:0;
    sel_t		gs;
    unsigned int		:0;
    sel_t		ldt;
    unsigned int		:0;
    unsigned int	t	:1,
    				:15,
			io_bmap	:16;
} tss_t;


extern tss_t tss;

void tss_init(void);
void tss_update(uint32_t esp);

#endif

