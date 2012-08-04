#ifndef C_OS_PCI_H
#define C_OS_PCI_H
#include <kernel/interrupt.h>
union pci_iobar {
    struct{
	unsigned region:1;
	unsigned reserved:1;
	unsigned base_addr:30;
	};
	uint32_t val;
} __attribute__((packed));

union pci_membar {
    struct{
	unsigned region:1;
	unsigned locatable:2;
	unsigned prefetch:1;
	unsigned base_addr:23;
	};
	uint32_t val;
} __attribute__((packed));
struct pci_conf_hdr {
	uint16_t vend_id;
	uint16_t dev_id;
	uint16_t command;
	uint16_t status;
	uint8_t rev;
	uint8_t class[3];
	uint8_t clg;
	uint8_t latency;
	uint8_t header;
	uint8_t bist;
	uint32_t bars[6];
	uint32_t reserved[2];
	uint32_t romaddr;
	uint32_t reserved2[2];
	uint8_t int_line;
	uint8_t int_pin;
	uint8_t mingnt;
	uint8_t max_lat;
	uint8_t data[192];
} __attribute__((packed));
//struct pci_device;
union cfg_addr2 {
    struct{
	unsigned type:2;
    unsigned reg:6;
    unsigned function:3;
    unsigned unit:5;
    unsigned bus:8;
    unsigned res:7;
    unsigned ecd:1;
	};
	uint32_t val;
} __attribute__((packed));
struct pci_device {
	union pci_dev_storage {
		struct pci_conf_hdr hdr;
		uint32_t storage[256/4];
	} header;
	union cfg_addr2 regs;
	intr_handler *irq_handler;
	struct pci_device *next;
	int device;
};

struct cfg_addr {
    unsigned type:2;
    unsigned reg:6;
    unsigned function:3;
    unsigned unit:5;
    unsigned bus:8;
    unsigned res:7;
    unsigned ecd:1;
} __attribute__((packed));


#define PCI_BAR_MEM  0x0
#define PCI_BAR_IO 	 0x1
#define PCI_BAR_NONE 0x3

typedef void pci_intr_handler (void *);
uint32_t pci_get_bar(struct pci_device *dev, uint8_t type);
void pci_reg_outw(struct pci_device *pci, uint8_t port, uint16_t val);
uint16_t pci_reg_inw(struct pci_device *pci, uint8_t port);
struct pci_device *pci_get_device(uint16_t vendor, uint16_t device);
void pci_register_irq(struct pci_device *pci, pci_intr_handler *handler, void *aux);
void pci_init();
#endif
