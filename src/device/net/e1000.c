/*	ChickenOS Intel Pro/1000 Network Card Driver
 *  Based on a driver by Joshua Cornutt
 *  Found here: http://www.randomaccessit.com/osdev/i825xx.c 
 *
 */





#include <stdio.h>
#include <mm/liballoc.h>
#include <device/pci.h>
#include <net/net_core.h>
#include <kernel/hw.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <kernel/memory.h>
#include <device/net/e1000.h>
struct e1000 *e1000_global;
#define E1000_DEV 0x100E
extern pagedir_t kernel_pd;
void pagedir_insert_page(pagedir_t pd, 
	virt_addr_t phys, virt_addr_t virt,uint8_t flags);

#define REG_CTRL 		0x0000
#define REG_EEPROM 		0x0014
#define REG_IMASK 		0x00D0
#define REG_RCTRL 		0x0100
#define REG_RXDESCLO  	0x2800
#define REG_RXDESCHI  	0x2804
#define REG_RXDESCLEN 	0x2808
#define REG_RXDESCHEAD 	0x2810
#define REG_RXDESCTAIL 	0x2818

#define REG_TCTRL 		0x0400
#define REG_TXDESCLO  	0x3800
#define REG_TXDESCHI  	0x3804
#define REG_TXDESCLEN 	0x3808
#define REG_TXDESCHEAD 	0x3810
#define REG_TXDESCTAIL 	0x3818



#define RCTRL_EN 	0x00000002
#define RCTRL_SBP 	0x00000004
#define RCTRL_UPE 	0x00000008
#define RCTRL_MPE 	0x00000010
#define RCTRL_8192 	0x00030000

#define ECTRL_FD   	0x01//FULL DUPLEX
#define ECTRL_ASDE 	0x20//auto speed enable
#define ECTRL_SLU  	0x40//set link up
void e1000_outl(struct e1000 *e, uint16_t addr, uint32_t val)
{
	outl(e->io_base, addr);
	outl(e->io_base + 4, val);
} 
void e1000_outb(struct e1000 *e, uint16_t addr, uint32_t val)
{
	outl(e->io_base, addr);
	outb(e->io_base + 4, val);
} 
uint32_t e1000_inl(struct e1000 *e, uint16_t addr)
{
	outl(e->io_base, addr);
	return inl(e->io_base + 4); 
}
uint32_t e1000_eeprom_read(struct e1000 *e, uint8_t addr)
{
	uint32_t val = 0;
	uint32_t test;
	if(e->is_e == 0)
		test = addr << 8;
	else
		test = addr << 2;

	e1000_outl(e, REG_EEPROM, test | 0x1);
	if(e->is_e == 0)
		while(!((val = e1000_inl(e, REG_EEPROM)) & (1<<4)))
		;//	printf("is %i val %x\n",e->is_e,val);
	else
		while(!((val = e1000_inl(e, REG_EEPROM)) & (1<<1)))
		;//	printf("is %i val %x\n",e->is_e,val);
	val >>= 16;
	return val;
}

void e1000_getmac(struct e1000 *e, char *mac)
{
	uint32_t temp;
	temp = e1000_eeprom_read(e, 0);
	mac[0] = temp &0xff;
	mac[1] = temp >> 8;
	temp = e1000_eeprom_read(e, 1);
	mac[2] = temp &0xff;
	mac[3] = temp >> 8;
	temp = e1000_eeprom_read(e, 2);
	mac[4] = temp &0xff;
	mac[5] = temp >> 8;
}
void e1000_linkup(struct e1000 *e)
{
	uint32_t val;
	val = e1000_inl(e,REG_CTRL);
	e1000_outl(e, REG_CTRL, val | ECTRL_SLU);
}
void e1000_interrupt_enable(struct e1000 *e)
{
	e1000_outl(e,REG_IMASK ,0x1F6DC);
	e1000_outl(e,REG_IMASK ,0xff & ~4);
	e1000_inl(e,0xc0);
}

size_t e1000_send(struct network_dev *dev, uint8_t *_buf, size_t length)
{
	struct e1000 *e = dev->device;
	e->tx_descs[e->tx_cur]->addr = (uint64_t)(uintptr_t)V2P(_buf);
	e->tx_descs[e->tx_cur]->length = length;
	e->tx_descs[e->tx_cur]->cmd = ((1 << 3) | 3);
	uint8_t old_cur = e->tx_cur;
	e->tx_cur = (e->tx_cur + 1) % NUM_TX_DESC;
	e1000_outl(e, REG_TXDESCTAIL, e->tx_cur);
	while(!(e->tx_descs[old_cur]->status & 0xff));
	return 0;
}
void e1000_received(struct e1000 *e)
{
	struct sockbuf *sb;
	uint16_t old_cur;
	while((e->rx_descs[e->rx_cur]->status & 0x1))
	{
		uint8_t *buf = (void *)(uintptr_t)P2V(e->rx_descs[e->rx_cur]->addr);
		uint16_t len = e->rx_descs[e->rx_cur]->length;
			
		sb = sockbuf_alloc(e->dev, len);
		kmemcpy(sb->data, buf, len);
		network_received(sb); 
		e->rx_descs[e->rx_cur]->status = 0;
		old_cur = e->rx_cur; 
		e->rx_cur = (e->rx_cur + 1) % NUM_RX_DESC;
		e1000_outl(e, REG_RXDESCTAIL, old_cur ) ;
	}
}

void e1000_eeprom_gettype(struct e1000 *e)
{
	uint32_t val = 0;
	e1000_outl(e, REG_EEPROM, 0x1); 
	
	for(int i = 0; i < 1000; i++)///while( val & 0x2 || val & 0x10)
	{
		val = e1000_inl(e, REG_EEPROM);
		if(val & 0x10)
			e->is_e = 0;
		else
			e->is_e = 1;	
	}
}
void e1000_handler(void *aux)
{
	struct e1000 *e = aux;
	uint32_t status = e1000_inl(e, 0xc0);
	
	if(status & 0x04)
	{
		e1000_linkup(e);
	}

	if(status & 0x10)
	{
		printf("threshold good\n");
	}


	if(status & 0x80)
	{
		e1000_received(e);
	}

//	printf("e1000 IRQ %x\n",status);
//	e1000_inl(e,0xc0);
}

void e1000_rxinit(struct e1000 *e)
{
	uintptr_t ptr;
	struct e1000_rx_desc *descs;
	ptr = (uintptr_t)(kmalloc(sizeof(struct e1000_rx_desc)*NUM_RX_DESC + 16));
	e->rx_free = (uint8_t *)ptr;
	if(ptr % 16 != 0)
		ptr = (ptr + 16) - (ptr % 16);
	descs = (struct e1000_rx_desc *)ptr;
	for(int i = 0; i < NUM_RX_DESC; i++)
	{
		e->rx_descs[i] = (struct e1000_rx_desc *)((uintptr_t)descs + i*16);
		e->rx_descs[i]->addr = (uint64_t)(uintptr_t)V2P(kmalloc(8192 + 16));
		e->rx_descs[i]->status = 0;
	}	
	
	//give the card the pointer to the descriptors
	e1000_outl(e, REG_RXDESCLO, V2P(ptr));
	e1000_outl(e, REG_RXDESCHI, 0);

	//now setup total length of descriptors
	e1000_outl(e, REG_RXDESCLEN, NUM_RX_DESC * 16);

	//setup numbers
	e1000_outl(e, REG_RXDESCHEAD, 0);
	e1000_outl(e, REG_RXDESCTAIL, NUM_RX_DESC);
	e->rx_cur = 0;
	
	//enable receiving
	//uint32_t flags = (2 << 16) | (1 << 25) | (1 << 26) | (1 << 15) | (1 << 5) | (0 << 8) | (1 << 4) | (1 << 3) | ( 1 << 2);
uint32_t flags = (2 << 16) | (1 << 25) | (1 << 26) | (1 << 15) | (1 << 5) | (0 << 8) | (0 << 4) | (0 << 3) | ( 1 << 2);
//	uint32_t flags = (2 << 16) | (1 << 25) | (1 << 26) | (1 << 15) | (1 << 5) | (0 << 8) | (1 << 4) | ( 1 << 2);
//	e1000_outl(e, REG_RCTRL, RCTRL_8192 | RCTRL_MPE);
	e1000_outl(e, REG_RCTRL, flags);//RCTRL_8192 | RCTRL_MPE | RCTRL_UPE |RCTRL_EN);
}
void e1000_txinit(struct e1000 *e)
{
	uintptr_t ptr;
	struct e1000_tx_desc *descs;
	ptr = (uintptr_t)(kmalloc(sizeof(struct e1000_tx_desc)*NUM_TX_DESC + 16));
	e->tx_free = (uint8_t *)ptr;
	if(ptr % 16 != 0)
		ptr = (ptr + 16) - (ptr % 16);
	descs = (struct e1000_tx_desc *)ptr;
	for(int i = 0; i < NUM_TX_DESC; i++)
	{
		e->tx_descs[i] = (struct e1000_tx_desc *)((uintptr_t)descs + i*16);
		e->tx_descs[i]->addr = 0;
		e->tx_descs[i]->cmd = 0;
	}	
	
	//give the card the pointer to the descriptors
	e1000_outl(e, REG_TXDESCLO, V2P(ptr));
	e1000_outl(e, REG_TXDESCHI, 0);

	//now setup total length of descriptors
	e1000_outl(e, REG_TXDESCLEN, NUM_TX_DESC * 16);

	//setup numbers
	e1000_outl(e, REG_TXDESCHEAD, 0);
	e1000_outl(e, REG_TXDESCTAIL, NUM_TX_DESC);
	e->tx_cur = 0;
	
	e1000_outl(e, REG_TCTRL, (1 << 1) | (1 << 3));
}

void e1000_start(struct e1000 *e)
{
	//set link up
	e1000_linkup(e);
	//have to clear out the multicast filter, otherwise shit breaks
	for(int i = 0; i < 0x80; i++)
		e1000_outl(e, 0x5200 + i*4, 0);

	e1000_interrupt_enable(e);

	e1000_rxinit(e);
	e1000_txinit(e);
}

struct network_dev *e1000_init()
{
	struct network_dev *device = kcalloc(sizeof(*device), 1);
	struct e1000 *e = (struct e1000 *)kmalloc(sizeof(*e));
	e1000_global = e;
	device->device = e;
	e->dev = device;
	
	e->pci = pci_get_device(INTEL_VEND, E1000_DEV);
	if(e->pci == NULL){
		e->pci = pci_get_device(INTEL_VEND, 0x109a);
	}
	if(e->pci == NULL){
		e->pci = pci_get_device(INTEL_VEND, 0x100f);
	}

	if(e->pci != NULL)
	{
		e->pci_hdr = e->pci->header;
		printf("Intel Pro/1000 Ethernet adapter Rev %i found at ", e->pci_hdr->rev);
		
		e->io_base = pci_get_bar(e->pci, PCI_BAR_IO) & ~1;
		printf("I/O base address %x\n",e->io_base);
		
		//e->mem_base = (uint8_t *)(pci_get_bar(e->pci, PCI_BAR_MEM) & ~3);
		//printf("mem base %x\n",e->mem_base);
		
		printf("IRQ %i PIN %i\n",e->pci_hdr->int_line, e->pci_hdr->int_pin);	

		e1000_eeprom_gettype(e);
		e1000_getmac(e, (char *)device->mac);
		print_mac((char *)&device->mac);
	
	//	for(int i = 0; i < 6; i++)
	//	e1000_outb(e,0x5400 + i, device->mac[i]);	
	
		pci_register_irq(e->pci, &e1000_handler, e);
		
		e1000_start(e);
		
		device->send = e1000_send;
	//	device->receive = e1000_receive;
		
		uint32_t flags = e1000_inl(e, REG_RCTRL);
		e1000_outl(e, REG_RCTRL, flags | RCTRL_EN);//RCTRL_8192 | RCTRL_MPE | RCTRL_UPE |RCTRL_EN);
	}
	else
	{
		kfree(e);
		kfree(device);
		e = NULL;
		device = NULL;
	}

	return device;

}


