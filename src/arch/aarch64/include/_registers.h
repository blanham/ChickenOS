#ifndef C_OS_ARCH_I386_REGISTERS_H
#define C_OS_ARCH_I386_REGISTERS_H
#include <stdint.h>

typedef struct {
	uintptr_t eip, useresp;
	//uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	//uint16_t ds, :16;
	//uint16_t es, :16;
	//uint16_t fs, :16;
	//uint16_t gs, :16;         	    // Data segment selectors
	//uint32_t int_no, err_code;		// Interrupt number and error code (if applicable)
	//uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
	int int_no;
} __attribute__((packed)) registers_t;


#define REGS_IP(x) ((x)->eip)
#define REGS_SP(x) ((x)->useresp)

void registers_dump(registers_t *regs);

#endif
