#ifndef C_OS_HW_H
#define C_OS_HW_H
#include <stdint.h>

typedef struct {
	uint16_t base;
} ioport_t;


void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

static inline void io_outb(ioport_t *io, uint16_t port, uint8_t value)
{
	outb(io->base + port, value);
}

static inline uint8_t io_inb(ioport_t *io, uint16_t port)
{
	return inb(io->base + port);
}

void io_outw(ioport_t *, uint16_t);
void io_outl(ioport_t *, uint32_t);


void shutdown();

#endif