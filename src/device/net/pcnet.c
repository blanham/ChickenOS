/*	ChickenOS AMD-PCNET Network Card Driver
 *	Broken in Virtualbox at the moment
 *
 */

#include <stdio.h>
#include <mm/liballoc.h>
#include <device/net/pcnet.h>
#include <device/pci.h>
#include <net/net_core.h>
#include <kernel/hw.h>
#include <mm/vm.h>
#include <kernel/memory.h>
//#include "rtl8139.h"
//http://code.google.com/searchframe#gufiwQeQ0iA/trunk/boot/u-boot-1.1.4/drivers/pcnet.c&q=pcnet&ct=rc&cd=3

#define PCNET_RDPW	0x10
#define PCNET_RAPW	0x12
#define PCNET_RSTW	0x14
#define PCNET_BDPW	0x16
#define PCNET_RDPL 	0x10
#define PCNET_RAPL	0x14
#define PCNET_RSTL	0x18
#define PCNET_BDPL	0x1c
//values taken from linux driver
#define PCNET_LOG_TX_BUFFERS	0
#define PCNET_LOG_RX_BUFFERS	2

#define TX_RING_SIZE		(1 << (PCNET_LOG_TX_BUFFERS))
#define TX_RING_LEN_BITS	((PCNET_LOG_TX_BUFFERS) << 12)

#define RX_RING_SIZE		(1 << (PCNET_LOG_RX_BUFFERS))
#define RX_RING_LEN_BITS	((PCNET_LOG_RX_BUFFERS) << 4)

#define PKT_BUF_SZ		1544
struct pcnet *global;

char packet[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x08, 0x00, 
0x27, 0x55, 0x8E, 0x0c, 0x08, 0x06, 0x00, 0x01, 
0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0xc0, 
0x9f, 0x50, 0x32, 0xa0, 0x80, 0x87, 0x9b, 0xcb, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x87, 
0x9b, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00 };
char pkt[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 
0x29, 0x53, 0x9d, 0x79, 0x08, 0x06, 0x00, 0x01, 
0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x0c, 
0x29, 0x53, 0x9d, 0x79, 0x0a, 0x78, 0x9b, 0x8c, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x78, 
0x9b, 0x01 };
void pcnet_csr_outw(struct pcnet *l, uint16_t addr, uint16_t val)
{
	pcnet_outl(l, PCNET_RAPW, addr);
	pcnet_outl(l, PCNET_RDPW, val); 
} 
uint16_t pcnet_csr_inw(struct pcnet *l, uint16_t addr)
{
	pcnet_outl(l, PCNET_RAPW, addr);
	return pcnet_inl(l, PCNET_RDPW); 
}
void pcnet_csr_outl(struct pcnet *l, uint16_t addr, uint32_t val)
{
	pcnet_outl(l, PCNET_RAPL, addr);
	pcnet_outl(l, PCNET_RDPL, val); 
} 
uint32_t pcnet_csr_inl(struct pcnet *l, uint16_t addr)
{
	pcnet_outl(l, PCNET_RAPL, addr);
	return pcnet_inl(l, PCNET_RDPL); 
}

void pcnet_bcr_outw(struct pcnet *l, uint16_t addr, uint16_t val)
{
	pcnet_outl(l, PCNET_RAPW, addr);
	pcnet_outl(l, PCNET_BDPW, val); 
} 
uint16_t pcnet_bcr_inw(struct pcnet *l, uint16_t addr)
{
	pcnet_outl(l, PCNET_RAPW, addr);
	return pcnet_inl(l, PCNET_BDPW); 
} 
void pcnet_bcr_outl(struct pcnet *l, uint16_t addr, uint32_t val)
{
	pcnet_outl(l, PCNET_RAPL, addr);
	pcnet_outl(l, PCNET_BDPL, val); 
} 
uint32_t pcnet_bcr_inl(struct pcnet *l, uint16_t addr)
{
	pcnet_outl(l, PCNET_RAPL, addr);
	return pcnet_inl(l, PCNET_BDPL); 
}
 
void pcnet_reset(struct pcnet *l)
{
/*	if(l->bit32)
		pcnet_inl(l, PCNET_RSTL);
	else*/
	{
		pcnet_inl(l, PCNET_RSTW);
		//pcnet_outl(l, PCNET_RSTW, 0);
	}
}
void pcnet_resetl(struct pcnet *l)
{
/*	if(l->bit32)
		pcnet_inl(l, PCNET_RSTL);
	else*/
	{
		pcnet_inl(l, PCNET_RSTW);
		//pcnet_outl(l, PCNET_RSTW, 0);
	}
}

size_t pcnet_send(struct network_dev *dev, uint8_t *_buf, size_t length)
{
	dev =dev;
	struct pcnet *l = dev->device;
//	interrupt_disable();	
	kmemcpy(l->tx_buffers[l->cur_tx], _buf, length);
	l->tx_descs[l->cur_tx].addr = (uintptr_t)V2P(l->tx_buffers[l->cur_tx]);
	l->tx_descs[l->cur_tx].flags2 = 0;
	l->tx_descs[l->cur_tx].status |= 0x8300;
	l->tx_descs[l->cur_tx].len =  -length;
//	printf("%p, io %x %x\n",l, &l->io_base, l->tx_descs[l->cur_tx].addr);
	uint16_t csr = pcnet_csr_inl(l, 0);
	pcnet_csr_outl(l, 0, csr | 8);
	l->cur_tx++;
	if(l->cur_tx == 8)
		l->cur_tx = 0;

//	interrupt_enable();	
	return length;
}
struct network_dev *temp_net;
void send_packet()//truct rtl8139 *rtl)
{
	//uint8_t *test = kmalloc(60);
//	kmemcpy(test, packet, 60);
//	kmemcpy(&test[6], global->mac,6);
//	pcnet_send(temp_net, (uint8_t*)test, 60);
//	kfree(test);
}

void pcnet_dumpregs(struct pcnet *l)
{
	for(int i = 0; i < 0x20; i++)
	{
		printf("%.2X:",pcnet_csr_inl(l, i));
		if(i %8 == 7)
			printf("\n");
	}
}
void pcnet_getmac(struct pcnet *l, char *mac)
{
	for(int i = 0; i < 6; i++)
	{
		mac[i] = pcnet_inb(l, i);
	}

}


void pcnet_receive(struct pcnet *l)
{
	size_t len;
	uint8_t *buf;
	struct sockbuf *sb;
	while((l->rx_descs[l->cur_rx].status & 0x8000) == 0)
	{
		if(!(l->rx_descs[l->cur_rx].status & 0x4000) &&
			(l->rx_descs[l->cur_rx].status & 0x0300) == 0x0300)
		{
			len = l->rx_descs[l->cur_rx].flags2 & 0xFFFF;
			buf = l->rx_buffers[l->cur_rx];
			sb = sockbuf_alloc(l->dev, len);
			kmemcpy(sb->data, buf, len);
			network_received(sb);


		}	
		l->rx_descs[l->cur_rx].addr = V2P(l->rx_buffers[l->cur_rx]);
		l->rx_descs[l->cur_rx].status = 0x8000;
		l->rx_descs[l->cur_rx].len = -2048;
		l->rx_descs[l->cur_rx].flags2 = 0;

		l->cur_rx++;
		if(l->cur_rx == 8)
			l->cur_rx = 0;
	}
}

void pcnet_handler(void *aux)
{
	struct pcnet *l = aux;
	uint16_t csr = pcnet_csr_inl(l, 0);
//	printf("LANCE IRQ CSR%x\n", csr);
/*	if(l->bit32 == 0){
		l->bit32 = 1;

	}else{*/
	if(!(csr & (0x1 << 7)))
	{
		printf("something\n");
		return;
	}
	if(csr & 0x0200){
	//	printf("Tx finished \n");

	}else if(csr & 0x0400){
		if(csr & 0x8000)
			printf("error: ");
//		printf("rx packet\n");

		pcnet_receive(l);

	}
//}
	pcnet_csr_outl(l, 0, csr);
}
void pcnet_handler_old(struct registers *regs)
{
	regs = regs;
	pcnet_handler(global);


}
void pcnet_start2(struct pcnet *l)
{
	//1) setup PCI io enable and possibly bus master
	/*  not necessary i think for virtualbox   */
	//2) get mac addr and store
	char mac[6];
	uint16_t val = 0;
	val = val;
	pci_register_irq(l->pci, &pcnet_handler, l);

	for(int i = 0; i < 0x40; i++)
		pcnet_csr_outl(l, i, 0);
	pcnet_csr_outl(l, 0, 4);
	//3) reset
	pcnet_reset(l);

	//4)enable 32bit mode
	pcnet_outl(l, 20, 0);
//	pcnet_bcr_outl(l, 20, 0x0102);
	pcnet_bcr_outl(l, 20, 0x0102);

	pcnet_getmac(l, (char *)&mac);
	kmemcpy(global->mac, mac, 6);
	print_mac((char *)mac);

	//5) Stop card
	pcnet_csr_outl(l, 0, 4);

	//6) setup descriptor table
	l->rx_descs = pallocn(4);//(void *)((uintptr_t)kcalloc(sizeof(struct pcnet_desc)+16,8) & ~0xf);
	l->tx_descs = pallocn(4);//(void *)((uintptr_t)kcalloc(sizeof(struct pcnet_desc)+16,8) & ~0xf);
	l->cur_rx = 0;
	l->cur_tx = 0;
	//7) fill table, flags2 in tx, and flags/2 in rx = 0, taken care of by calloc
	void *buf;
	for(int i = 0; i < 8; i++)
	{
		buf = palloc();//kmalloc(2048);
		l->rx_buffers[i] = buf;
		l->rx_descs[i].addr = V2P(buf);
		l->rx_descs[i].len = -2048;//0x7FF | 0xf000;
		l->rx_descs[i].status = 0x8000;
		l->rx_descs[i].flags2 = 0;
		buf = palloc();//kmalloc(2048);
		l->tx_buffers[i] = buf;
		l->tx_descs[i].addr = 0;//V2P(buf);
		l->tx_descs[i].len = 0;
		l->tx_descs[i].status = 0;
		l->tx_descs[i].flags2 = 0;
	}
	//8) set up init block
	l->init = kcalloc(sizeof(*(l->init)), 1);
	l->init->mode = 0x0000;// | 0x3 << 7;//currently set to promiscuous	
	l->init->rx_len = 3;
	l->init->tx_len = 3;
	for(int i =0; i < 6; i++)
		l->init->mac[i] = mac[i];
	l->init->rx_desc = V2P(l->rx_descs);
	l->init->tx_desc = V2P(l->tx_descs);
//	l->init->filter = 0xffffffff;
//	l->init->filter2 = 0xffffffff;
	//9) install init pointer
	uintptr_t initptr = V2P(l->init);
	pcnet_csr_outl(l, 1, initptr & 0xFFFF);
	pcnet_csr_outl(l, 2, initptr >> 16);
	//10) final setup
//	pcnet_bcr_outl(l, 9, 1 << 2);
	pcnet_csr_outl(l, 0, 0x1);
	int i;
	for(i = 100000; i > 0; i--)
	{
		if(pcnet_csr_inl(l,0) & 0x100)
			break;

	}
//	pcnet_csr_outl(l, 0, 0x2);
	pcnet_csr_outl(l, 0, 0x42);
	pcnet_dumpregs(l);
	val = pcnet_csr_inl(l, 4);
	pcnet_csr_outl(l, 4, val | 0xC00);
}
struct network_dev * pcnet_init()
{
	struct network_dev *device = kmalloc(sizeof(*device));
	struct pcnet *l = (struct pcnet *)kmalloc(sizeof(*l));
	global = l;
	temp_net = device;
	device->device = l;
	l->dev = device;
	l->pci = pci_get_device(AMD_VEND, PCNET_DEV);
//	l->rcv_buffer = kmalloc((8192*8)+16+1500);
//	l->tx_buffers = (void *)P2V(0x3380000);//kmalloc((8192+16+1500)*4);
	if(l->pci != NULL)
	{
		l->bit32 = 0;
		l->pci_hdr = l->pci->header;
		printf("AMD PCNET Ethernet adapter found Rev %i IRQ %i\n", l->pci_hdr->rev, l->pci_hdr->int_line);
		
		l->io_base = pci_get_bar(l->pci, PCI_BAR_IO) & ~1;
		printf("io base %x\n",l->io_base);
		
		l->mem_base = (uint8_t *)(pci_get_bar(l->pci, PCI_BAR_MEM) & ~3);
		printf("mem base %x\n",l->mem_base);
//uint16_t out = pci_reg_inw(l->pci, 4);
//	pci_reg_outw(l->pci, 4, out|4|1);
		pcnet_start2(l);
	//	pcnet_dumpregs(l);
		pcnet_getmac(l,(char *)&device->mac);
		print_mac((char *)l->mac);
		device->send = pcnet_send;
		//device->receive = pcnet_receive;

	}
	else
	{
		kfree(l);

		l = NULL;
		return NULL;
	}

	return device;

}

