#include <chicken/device/ioport.h>

// XXX: These are only here to placate drivers that need them
//		Remove these once build system is improved
void outb(uint16_t port, uint8_t value)
{
	(void)port;
	(void)value;
}

void outw(uint16_t port, uint16_t value)
{
	(void)port;
	(void)value;
}

void outl(uint16_t port, uint32_t value)
{
	(void)port;
	(void)value;
}

uint8_t inb(uint16_t port)
{
	(void)port;
	return 0;
}

uint16_t inw(uint16_t port)
{
	(void)port;
	return 0;
}

uint32_t inl(uint16_t port)
{
	(void)port;
	return 0;
}

// TODO: move this elsewhere
/* shutsdown qemu/bochs */
void shutdown()
{
	//const char s[] = "Shutdown";
	//const char *p;
	//for (p = s; *p != '\0'; p++)
	//	outb (0x8900, *p);

	//// For QEMU
	//outw(0x604, 0x2000);
	//while(1)
	//asm volatile ("hlt");
}