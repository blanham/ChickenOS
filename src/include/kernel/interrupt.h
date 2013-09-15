#ifndef C_OS_INTERRUPT_H
#define C_OS_INTERRUPT_H

#include <stdint.h>

enum {
	IRQ0 = 32, IRQ1, IRQ2, IRQ3,
	IRQ4, IRQ5, IRQ6, IRQ7,
	IRQ8, IRQ9, IRQ10, IRQ11,
	IRQ12, IRQ13, IRQ14, IRQ15
}; 

enum { PIT_I = IRQ0, KBD_I, CASC_I, COM2_I, COM1_I, FD_I};
enum intr_status {INTR_ENABLED, INTR_DISABLED};

typedef struct registers
{
  	uint32_t ESP; //try to get rid of this
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	uint16_t ds,:16;
	uint16_t es, :16;
	uint16_t fs, :16;
	uint16_t gs, :16;                  // Data segment selectors
	uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
	uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} __attribute__((packed)) registers_t;

/*TODO:rename me to registers_dump*/
void dump_regs(registers_t *regs);

typedef void intr_handler (struct registers *);
void interrupt_init();
void interrupt_register(int irq, intr_handler *handler);


enum intr_status interrupt_disable();
enum intr_status interrupt_enable();
enum intr_status interrupt_set(enum intr_status);
enum intr_status interrupt_get();
#endif
