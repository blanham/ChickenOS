#ifndef C_OS_INTERRUPT_H
#define C_OS_INTERRUPT_H

#include <arch/i386/interrupt.h>
#include <stdint.h>

enum intr_status {INTR_ENABLED, INTR_DISABLED};


typedef void intr_handler (registers_t *);
void interrupt_init();
void interrupt_register(int irq, intr_handler *handler);


enum intr_status interrupt_disable();
enum intr_status interrupt_enable();
enum intr_status interrupt_set(enum intr_status);
enum intr_status interrupt_get();
#endif
