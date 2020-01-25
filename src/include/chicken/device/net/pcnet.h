#ifndef C_OS_DEVICE_NET_LANCE_H
#define C_OS_DEVICE_NET_LANCE_H

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

#define AMD_VEND 0x1022
#define PCNET_DEV 0x2000

struct network_dev * pcnet_init();

#endif