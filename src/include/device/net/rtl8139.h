#ifndef C_OS_DEVICE_NET_RTL8139_H
#define C_OS_DEVICE_NET_RTL8139_H
struct rtl8139 {
	struct pci_device *pci;
	struct pci_conf_hdr *pci_hdr;
	uint16_t io_base;
	uint8_t *mem_base;
	uint8_t  mac[6];
	uint8_t *rcv_buffer;
	uint8_t *tx_buffers;
	int tx_cur;
};



struct network_dev * rtl8139_init();
#endif
