#include <common.h>
#include <thread/syscall.h>
#include <arch/i386/interrupt.h>

void i386_syscall_handler(registers_t *regs)
{
	syscall_handler(regs);
}
