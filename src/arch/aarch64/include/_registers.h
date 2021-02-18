#ifndef C_OS_ARCH_AARCH64_REGISTERS_H
#define C_OS_ARCH_AARCH64_REGISTERS_H
#include <stdint.h>
typedef struct {
	uintptr_t instruction_pointer, useresp;
	//uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	//uint16_t ds, :16;
	//uint16_t es, :16;
	//uint16_t fs, :16;
	//uint16_t gs, :16;         	    // Data segment selectors
	//uint32_t int_no, err_code;		// Interrupt number and error code (if applicable)
	//uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
	uint64_t arg0, arg1, arg2, arg3, arg4, arg5, stack_pointer, return_value; // Pushed by pusha.
	int int_no;
} __attribute__((packed)) registers_t;


typedef struct {
	uintptr_t instruction_pointer, useresp;
	//uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	//uint16_t ds, :16;
	//uint16_t es, :16;
	//uint16_t fs, :16;
	//uint16_t gs, :16;         	    // Data segment selectors
	//uint32_t int_no, err_code;		// Interrupt number and error code (if applicable)
	//uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
	uint64_t arg0, arg1, arg2, arg3, arg4, arg5, stack_pointer, return_value; // Pushed by pusha.
	int int_no;
} __attribute__((packed)) registers2_t;


#define REGS_IP(x) ((x)->instruction_pointer)
#define REGS_SP(x) ((x)->stack_pointer)

void registers_dump(registers_t *regs);

#endif
