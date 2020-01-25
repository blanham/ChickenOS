#include <device/pci.h>
#include <kernel/hw.h>
#include <stdio.h>
#include <kernel/memory.h>
#include <mm/liballoc.h>

struct pci_device *pci_device_list;

struct pci_irq {
	pci_intr_handler *handler;
	int int_pin;
	void *aux;
} pci_irqs[32][4];

#define PCI_ADDRESS 0xCF8
#define PCI_DATA	0xCFC

void pci_reg_outw(struct pci_device *pci, uint8_t port, uint16_t val)
{
	union cfg_addr2 saddr;
	saddr.val = pci->regs.val;
	saddr.reg = port;
	outl(PCI_ADDRESS, saddr.val);
	outw(PCI_DATA, val);
}

uint16_t pci_reg_inw(struct pci_device *pci, uint8_t port)
{
	union cfg_addr2 saddr;
	saddr.val = pci->regs.val;
	saddr.reg = port;
	outl(PCI_ADDRESS, saddr.val);
	return inw(PCI_DATA);
}

void print_cfg(union cfg_addr2 *a)
{
	printf("type %u reg %x function %u unit %x bus %x res %u ecd %u\n",
		a->type,a->reg,a->function,a->unit,a->bus,a->res,a->ecd);
}

uint32_t pci_device_read_config(struct pci_conf_hdr **hdr, int bus, int device, int function)
{
	union cfg_addr2 saddr;
	saddr.val = 0;
	saddr.bus = bus;
	saddr.unit = device;
	saddr.function = function;
	saddr.ecd = 1;
	*hdr = (struct pci_conf_hdr *)kmalloc(sizeof(struct pci_conf_hdr));

	for (saddr.reg = 0; saddr.reg < 0x3f; saddr.reg++) {
		outl(PCI_ADDRESS, saddr.val);
		((uint32_t *)*hdr)[saddr.reg] = inl(PCI_DATA);
	}

	saddr.reg = 0;

	return saddr.val;
}

void pci_device_install(uint32_t val, struct pci_conf_hdr *header)
{
	struct pci_device *iter = NULL;
	struct pci_device *new = (struct pci_device *)kmalloc(sizeof(*new));

	new->header = header;
	new->regs.val = val;
	new->device = ((union cfg_addr2)val).function;

	if (pci_device_list == NULL) {
		pci_device_list = new;
		pci_device_list->next = NULL;
	} else {
		for (iter = pci_device_list; iter->next != NULL; iter = iter->next);
		iter->next = new;
		new->next = NULL;
	}
}

void pci_list()
{
	struct pci_device *iter = pci_device_list;
	while (iter != NULL) {
		printf("Found PCI device %.2i %.4X %.4X IRQ %2i Function %X MajMin %2X %2X %2X\n",
				iter->device, iter->header->vend_id,iter->header->dev_id,
				iter->header->int_line, iter->regs.function, iter->header->pci_major,
				iter->header->pci_minor, iter->header->pci_interface);
		iter = iter->next;
	}

}

void pci_handler(registers_t *regs)
{
	int int_no = regs->int_no - 32;

	for (int i = 0; i < 4; i++) {
		if (pci_irqs[int_no][i].handler != NULL)
			pci_irqs[int_no][i].handler(pci_irqs[int_no][i].aux);

	}
}

void pci_register_irq(struct pci_device *pci, pci_intr_handler *handler, void *aux)
{
	int int_line = pci->header->int_line;
	int int_pin = pci->header->int_pin;
	//TODO: make this a list later
	struct pci_irq *irq = &pci_irqs[int_line][int_pin];

	irq->handler = handler;
	irq->int_pin = int_pin;
	irq->aux = aux;
	interrupt_register(32 + int_line, &pci_handler);
}

void pci_bus_scan(int bus)
{

	uint32_t ret;
	struct pci_conf_hdr *hdr;

	//printf("Scanning PCI bus %i\n",bus);

	for (int i = 0; i <  0x20; i++) {
		ret =  pci_device_read_config(&hdr, bus, i, 0);

		if(hdr->vend_id == 0xFFFF) {
			kfree(hdr);
			continue;
		}

		pci_device_install(ret, hdr);

		if ((hdr->header & 0x80) != 0) {
			for (int j = 1; j < 8; j++) {
				ret =  pci_device_read_config(&hdr, bus, i, j);

				if (hdr->vend_id == 0xFFFF) {
					kfree(hdr);
					continue;
				}

				pci_device_install(ret, hdr);

			}
		}
	}
}

struct pci_device *pci_get_device(uint16_t vendor, uint16_t device)
{
	struct pci_device *ret = pci_device_list;
	for(; ret != NULL; ret = ret->next)
		if(ret->header->dev_id == device && ret->header->vend_id == vendor)
			break;
	return ret;
}

uint32_t pci_get_bar(struct pci_device *dev, uint8_t type)
//uint32_t pci_get_bar_by_type(struct pci_device *dev, uint8_t type)
{
	uint32_t bar = 0;
	for(int i = 0; i < 6; i++)
	{
		bar = dev->header->bars[i];
		if((bar & 0x1) == type) // XXX: ???
			return bar;
	}

	return 0xFFFFFFFF;
}

uint32_t pci_get_barn(struct pci_device *dev, uint8_t type, uint8_t num)
{
	uint32_t bar = 0;
//	for(int i = 0; i < 6; i++)
	{
		bar = dev->header->bars[num];
		if((bar & 0x1) == type)
			return bar;

		if (bar == 0)
			return 0;
	}

	return 0xFFFFFFFF;
}

void pci_init()
{
	union cfg_addr2 saddr,saddr2;
	saddr.val = 0;
	saddr.ecd = 1;

	for(int i = 0; i < 0xFF; i++)
	{
		saddr.bus = i;
		outl(PCI_ADDRESS, saddr.val);
		saddr2.val = inl(PCI_DATA);
		if(saddr2.val == 0xFFFFFFFF)
			continue;
		pci_bus_scan(i);
	}
	//pci_list();
}
