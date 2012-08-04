#ifndef C_OS_DEVICE_NET_RTL8139_H
#define C_OS_DEVICE_NET_RTL8139_H
struct rtl8139 {
	struct pci_device *pci;
	struct pci_conf_hdr *pci_hdr;
	struct network_dev *dev;
	uint16_t io_base;
	uint8_t *mem_base;
	uint8_t  mac[6];
	uint8_t *rx_buffer;
	uint8_t *tx_buffers;
	int tx_cur;
	uintptr_t rx_offset;
};



struct network_dev * rtl8139_init();
#endif
