/*	ChickenOS - interrupt.c
 */
#include <common.h>
#include <stdio.h>
#include <chicken/thread.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <device/console.h>

static enum intr_status interrupt_status = INTR_DISABLED;

enum intr_status interrupt_get()
{
	return interrupt_status;
}

enum intr_status interrupt_disable()
{
	enum intr_status old = interrupt_get();

	arch_interrupt_disable();
	interrupt_status = INTR_DISABLED;

	return old;
}

enum intr_status interrupt_enable()
{
	enum intr_status old = interrupt_get();

	arch_interrupt_enable();
	interrupt_status = INTR_ENABLED;

	return old;
}

enum intr_status interrupt_set(enum intr_status status)
{
	if(status == INTR_ENABLED)
		return interrupt_enable();
	else
		return interrupt_disable();
}

void interrupt_init()
{
	interrupt_status = INTR_DISABLED;
	arch_interrupt_init();
}
