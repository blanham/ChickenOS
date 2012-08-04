#ifndef C_OS_DEVICE_NET_LANCE_H
#define C_OS_DEVICE_NET_LANCE_H
#include <kernel/hw.h>

struct pcnet_desc {
	uint32_t addr;
	int16_t len;
	uint16_t status;
//	uint32_t flags;
	uint32_t flags2;
	uint32_t virt;
} __attribute__((packed));
struct pcnet_init {
	uint16_t mode;
	unsigned pad:4;
	unsigned rx_len:4;
	unsigned pad2:4;
	unsigned tx_len:4;
	uint8_t  mac[6];
//	unsigned long long mac:48;
	uint16_t reserved;
	uint32_t filter;
	uint32_t filter2;
	uint32_t rx_desc;
	uint32_t tx_desc;
//	uint32_t reserved2;
} __attribute__((packed));
struct pcnet {
	struct pci_device *pci;
	struct pci_conf_hdr *pci_hdr;
	struct network_dev *dev;
	uint16_t io_base;
	uint8_t *mem_base;
	uint8_t  mac[6];
	uint8_t cur_rx;
	uint8_t cur_tx;
	struct pcnet_desc *rx_descs;
	struct pcnet_desc *tx_descs;
	uint8_t *rx_buffers[8];
	uint8_t *tx_buffers[8];
	struct pcnet_init *init;
	int bit32;
};


static inline uint8_t pcnet_inb(struct pcnet *l, uint8_t port)
{
	return inb(l->io_base + port);
}
static inline uint16_t pcnet_inw(struct pcnet *l, uint8_t port)
{
	return inw(l->io_base + port);
}

static inline uint32_t pcnet_inl(struct pcnet *l, uint8_t port)
{
	return inl(l->io_base + port);
}

static inline void pcnet_outb(struct pcnet *l, uint8_t port, uint8_t value)
{
	outb(l->io_base + port, value);
}

static inline void pcnet_outw(struct pcnet *l, uint8_t port, uint16_t value)
{
	outw(l->io_base + port, value);
}

static inline void pcnet_outl(struct pcnet *l, uint8_t port, uint32_t value)
{
	outl(l->io_base + port, value);
}


struct network_dev * pcnet_init();
#define AMD_VEND 0x1022
#define PCNET_DEV 0x2000
#endif
