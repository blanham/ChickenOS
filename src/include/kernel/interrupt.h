#ifndef C_OS_INTERRUPT_H
#define C_OS_INTERRUPT_H

#ifdef ARCH_I386
#include <arch/i386/interrupt.h>
#elif ARCH_ARM
#include <arch/arm/interrupt.h>
#endif

#include <stdint.h>

enum intr_status { INTR_DISABLED, INTR_ENABLED};


typedef void intr_handler (registers_t *);
void interrupt_init();
void interrupt_register(int irq, intr_handler *handler);

void arch_interrupt_enable();
void arch_interrupt_disable();
void arch_interrupt_init();



enum intr_status interrupt_disable();
enum intr_status interrupt_enable();
enum intr_status interrupt_set(enum intr_status);
enum intr_status interrupt_get();
#endif
