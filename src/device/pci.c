#include <device/pci.h>
#include <kernel/hw.h>
#include <stdio.h>
#include <kernel/memory.h>
#include <mm/liballoc.h>

struct pci_device *pci_device_list;

#define PCI_ADDRESS 0xCF8
#define PCI_DATA	0xCFC
void print_cfg(union cfg_addr2 *a)
{
	printf("type %u reg %x function %u unit %x bus %x res %u ecd %u\n",
		a->type,a->reg,a->function,a->unit,a->bus,a->res,a->ecd);
}
struct pci_irq {
	pci_intr_handler *handler;
	int int_pin;
	void *aux;
} pci_irqs[32][4];

/*
static void pci_interrupt(struct registers *regs)
{
	regs = regs;
	printf("PCI IRQ\n");


}*/
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

void pci_device_install(int bus, int device)
{
	union cfg_addr2 saddr;
	saddr.val = 0;
	saddr.bus = bus;
	saddr.ecd = 1;
	saddr.unit = device;
	struct pci_device *iter = NULL;
	struct pci_device *new = (struct pci_device *)kmalloc(sizeof(*new));
	for(saddr.reg = 0; saddr.reg < 0x3f; saddr.reg++)
	{
		outl(PCI_ADDRESS, saddr.val);
		new->header.storage[saddr.reg] = inl(PCI_DATA);	
	}
	saddr.reg = 0;
	new->regs.val = saddr.val;
	if(pci_device_list == NULL)
	{
		pci_device_list = new;
		pci_device_list->next = NULL;
	}
	else
	{
		for(iter = pci_device_list; iter->next != NULL; iter = iter->next);
		iter->next = new;
		new->next = NULL;
	}

	printf("Found PCI device %X %X IRQ %i\n",new->header.hdr.vend_id,new->header.hdr.dev_id, new->header.hdr.int_line);
}
void pci_handler(struct registers *regs)
{
	int int_no = regs->int_no - 32;
	for(int i = 0; i < 4; i++)
	{
		if(	pci_irqs[int_no][i].handler != NULL)
			pci_irqs[int_no][i].handler(pci_irqs[int_no][i].aux);
	}
}
void pci_register_irq(struct pci_device *pci, pci_intr_handler *handler, void *aux)
{
	int int_line = pci->header.hdr.int_line;
	int int_pin = pci->header.hdr.int_pin;
	//TODO: make this a list later
	struct pci_irq *irq = &pci_irqs[int_line][int_pin];

	irq->handler = handler;
	irq->int_pin = int_pin;
	irq->aux = aux;
	interrupt_register(32 + int_line, &pci_handler);
}

void pci_bus_scan(int bus)
{

	union cfg_addr2 saddr, saddr2;
	saddr.val = 0;
	saddr.bus = bus;
	saddr.ecd = 1;
	printf("Scanning PCI bus %i\n",bus);
	for(int i = 0; i <  0x20; i++)
	{
		saddr.unit = i;
		outl(PCI_ADDRESS, saddr.val);
		saddr2.val = inl(PCI_DATA);
		if(saddr2.val == 0xFFFFFFFF)
			continue;
		pci_device_install(bus, i);
	}
}

struct pci_device *pci_get_device(uint16_t vendor, uint16_t device)
{
	struct pci_device *iter = pci_device_list;
	for(;iter != NULL; iter = iter->next)
	{
		if(iter->header.hdr.dev_id == device && iter->header.hdr.vend_id == vendor)
			return iter;;
	}
	return NULL;
}
uint32_t pci_get_bar(struct pci_device *dev, uint8_t type)
{
	uint32_t bar = 0;
	for(int i = 0; i < 6; i++)
	{
		bar = dev->header.hdr.bars[i];
		if((bar & 0x1) == type)
			return bar;

	}

	return 0xFFFFFFFF;
}

void pci_init()
{
	union cfg_addr2 saddr,saddr2;
	saddr.val = 0;
	saddr.ecd = 1;
	//interrupt_register(int irq, intr_handler *handler)

	for(int i = 0; i < 0xFF; i++)
	{
		saddr.bus = i;
		outl(PCI_ADDRESS, saddr.val);
		saddr2.val = inl(PCI_DATA);
		if(saddr2.val == 0xFFFFFFFF)
			continue;
		pci_bus_scan(i);
	}
//struct pci_device *get = pci_get_device(0x10ec,0x8139);
//	printf("%x\n",get);
}
