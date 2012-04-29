#ifndef C_OS_DEV_NET_RTL8139_H
#define C_OS_DEV_NET_RTL8139_H

#define RTL8139_VEND 0x10EC
#define RTL8139_DEV  0x8139
enum rtl_registers {
	MAC0, MAC1, MAC2,
	MAC3, MAC4, MAC5,
	MAR0 = 8, MAR1, MAR2,
	MAR3, MAR4, MAR5,
	MAR6, MAR7,
	RBSTART = 0x30,
	CMD = 0x37,
	IMR = 0x3c,
	ISR = 0x3e,
	CONFIG_1 = 0x52

};
uint32_t test_values[] = {
0x509fc000, 0x0000a032, 0x80000000, 0x40000004, 0x0008a042, 0x0008a042, 0x0008a042, 0x0008a042,
0x0d17a000, 0x0d17a600, 0x0d17ac00, 0x0d17b200, 0x0d140000, 0x0d0a0000, 0x63ac639c, 0x0000c07f,
0x77400680, 0x0000f78e, 0x043b99c1, 0x00000000, 0x008d10c6, 0x00000000, 0x0088c510, 0x00100000,
0x1100f00f, 0x01e1782d, 0x000145e1, 0x00000000, 0x00000704, 0x000207c8, 0x60f60c59, 0x7b732660


};



static inline uint8_t rtl_inb(struct rtl8139 *rtl, uint8_t port)
{
	return inb(rtl->io_base + port);
}
static inline uint16_t rtl_inw(struct rtl8139 *rtl, uint8_t port)
{
	return inw(rtl->io_base + port);
}

static inline uint32_t rtl_inl(struct rtl8139 *rtl, uint8_t port)
{
	return inl(rtl->io_base + port);
}

static inline void rtl_outb(struct rtl8139 *rtl, uint8_t port, uint8_t value)
{
	outb(rtl->io_base + port, value);
}

static inline void rtl_outw(struct rtl8139 *rtl, uint8_t port, uint16_t value)
{
	outw(rtl->io_base + port, value);
}

static inline void rtl_outl(struct rtl8139 *rtl, uint8_t port, uint32_t value)
{
	outl(rtl->io_base + port, value);
}
#endif
