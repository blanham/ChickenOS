#include <kernel/hw.h>

/* taken from: http://www.jamesmolloy.co.uk/ */
void outb(uint16_t port, uint8_t value)
{
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}
void outw(uint16_t port, uint16_t value)
{
	asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

void outl(uint16_t port, uint32_t value)
{
	asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port)
{
	unsigned char ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint16_t inw(uint16_t port)
{
	unsigned short ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint32_t inl(uint16_t port)
{
	uint32_t ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

/* shutsdown qemu/bochs */
void shutdown()
{
	const char s[] = "Shutdown";
	const char *p;
	for (p = s; *p != '\0'; p++)
		outb (0x8900, *p);
	asm volatile ("hlt");
}

