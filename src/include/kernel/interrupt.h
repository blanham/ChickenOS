#ifndef C_OS_INTERRUPT_H
#define C_OS_INTERRUPT_H

#include <kernel/types.h>
enum {
	IRQ0 = 32, IRQ1, IRQ2, IRQ3,
	IRQ4, IRQ5, IRQ6, IRQ7,
	IRQ8, IRQ9, IRQ10, IRQ11,
	IRQ12, IRQ13, IRQ14, IRQ15
}; 
struct registers
{
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   uint32_t ds, es, fs, gs;                  // Data segment selectors
   uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
   uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} __attribute__((packed));

typedef struct registers registers_t;

void dump_regs(struct registers *regs);
void interrupt_init();

typedef void intr_handler (struct registers *);
void interrupt_register(int irq, intr_handler *handler);

enum { PTI_I, KBD_I = IRQ1, CASC_I, COM2_I, COM1_I, FD_I};
#endif
