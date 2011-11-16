#ifndef C_OS_HW_H
#define C_OS_HW_H
#include <kernel/types.h>
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

void shutdown();
#endif
