#ifndef C_OS_ARCH_I386_INTERRUPT_H
#define C_OS_ARCH_I386_INTERRUPT_H
#include <stdint.h>

#define NUM_IRQS 16
#define NUM_ISRS 32

enum {
	IRQ0 = 32, IRQ1, IRQ2, IRQ3,
	IRQ4, IRQ5, IRQ6, IRQ7,
	IRQ8, IRQ9, IRQ10, IRQ11,
	IRQ12, IRQ13, IRQ14, IRQ15
};

typedef struct registers {
  	uint32_t ESP; // try to get rid of this
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	uint16_t ds, :16;
	uint16_t es, :16;
	uint16_t fs, :16;
	uint16_t gs, :16;         	    // Data segment selectors
	uint32_t int_no, err_code;		// Interrupt number and error code (if applicable)
	uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} __attribute__((packed)) registers_t;

void dump_regs(registers_t *regs);
void arch_interrupt_init();

#endif
