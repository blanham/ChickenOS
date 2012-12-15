#include <stdio.h>
#include <mm/liballoc.h>
#include <device/net/rtl8139.h>
#include <device/pci.h>
#include <net/net_core.h>
#include <kernel/hw.h>
#include <kernel/vm.h>
#include <kernel/memory.h>
#include "rtl8139.h"

struct rtl8139 *global;
#define RX_BUF_LEN 8192
#define RX_BUF_PAD 16
void rtl_receive(struct rtl8139 *rtl)
{
	uint16_t status, len;
	int16_t offset;
	struct sockbuf *sb;
	while(!(rtl_inw(rtl, CMD) & 0x1))
	{
		//now check the rx_status from the packet
		status = *(uint16_t *)(rtl->rx_buffer + rtl->rx_offset);
		len = *(uint16_t *)(rtl->rx_buffer + rtl->rx_offset + 2);
		if(status & 0x1)
		{
		//	printf("received packet! status %X len %X\n", status, len);
			//need to have function that allocates this for us
			sb = sockbuf_alloc(rtl->dev,len);
			if(sb == NULL)
			{
				printf("out of memory error in %s\n",__func__);
				break;
			}
			//have to account for wrapping the buffer
			offset = rtl->rx_offset + 4 + len - RX_BUF_LEN;

			if(offset < 0)
				offset = 0;
			kmemcpy(sb->data, rtl->rx_buffer + rtl->rx_offset + 4, len - offset);
			//uintptr_t off2 = len - offset;
			kmemcpy(sb->data + (len - offset), rtl->rx_buffer, offset);
			network_received(sb); 
			
		}else{

			printf("receive error\n");
		}
		rtl->rx_offset = ((rtl->rx_offset + 3 + 4 + len) & ~3) % RX_BUF_LEN;
		rtl_outw(rtl, 0x38, rtl->rx_offset -16);//set receive pointer
	}


}

void rtl_handler(void *aux)
{
	struct rtl8139 *rtl = aux;
	uint16_t irq = rtl_inw(rtl, ISR);
	if(irq & 0x20){
		rtl_outw(rtl, ISR, 0x20);
	}
	if(irq & 0x10){

		rtl_outw(rtl, ISR, 0x10);
	}
	if(irq & 0x4){
		rtl_outw(rtl, ISR, 0x4);
		//printf("tx\n");
	}

	if(irq & 0x1)
	{	
		
		rtl_receive(rtl);

		rtl_outw(rtl, ISR, 0x1);
	}
}

char bufbuf[8192+16+1500];
void rtl8139_enable_timer(struct rtl8139 *rtl)
{
	rtl_outl(rtl, 0x54, 32768);//set rate ~1ms
	rtl_outl(rtl, 0x48, 0); //set timer count to 0
}
char lpacket[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xc0, 
0x9f, 0x50, 0x32, 0xa0, 0x08, 0x06, 0x00, 0x01, 
0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0xc0, 
0x9f, 0x50, 0x32, 0xa0, 0x80, 0x87, 0x9b, 0xcb, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x87, 
0x9b, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00 };


size_t rtl8139_send(struct network_dev *dev, uint8_t *_buf, size_t length)
{
	struct rtl8139 *rtl = dev->device;
	rtl = rtl;
	_buf = _buf;
	length = length;
	
	void* tx_buffer = (void *)(rtl->tx_buffers + 8192*rtl->tx_cur);
	kmemset(tx_buffer, 0, (length <60) ? 60 : length);
	kmemcpy(tx_buffer, _buf, length);
		
	if(length < 60)
		length = 60;
	
	rtl_outl(rtl, 0x20 + rtl->tx_cur*4, V2P(tx_buffer));
	rtl_outl(rtl, 0x10 + rtl->tx_cur*4, length | (48 << 16)); 
	rtl->tx_cur++;
	rtl->tx_cur %= 4;

	return length;
}
void rtl8139_dumpregs(struct rtl8139 *rtl)
{

		for(int i = 0; i < 0x80/4; i++)
		{
			printf("%.8X:",rtl_inl(rtl, i*4));
			if(i %8 == 7)
				printf("\n");
		}
}
void rtl8139_start(struct rtl8139 *rtl)
{
	rtl->tx_cur = 0;
	rtl_outb(rtl, 0x37, 0x10);
	while((rtl_inb(rtl, 0x37) & 0x10) != 0);
	
	kmemset(rtl->rx_buffer, 0, (8192*8)+16+1500);
	rtl_outl(rtl, 0x30,(uintptr_t)V2P(rtl->rx_buffer));
	rtl_outb(rtl, 0x37, 0xc);
	
	for(int i=0; i < 4; i++)
	{
		rtl_outl(rtl, 0x20 + i*4, (uintptr_t)V2P(rtl->tx_buffers) + i*(8192 +16+1500));
	}
	//TODO: need to register pci IRQs instead of doing it directly
	//interrupt_register(32 + rtl->pci_hdr->int_line, &rtl_handler);

	pci_register_irq(rtl->pci, &rtl_handler, rtl);
	rtl_outl(rtl, 0x44, (1 << 7) | 8|  (1 << 1));
	rtl_outw(rtl, 0x3c, 0x5 );
	for(int i = 0; i < 6; i ++)
		rtl->mac[i] = rtl_inb(rtl, i);
	for(int i = 0; i < 100; i++)
	{
	/*	uint16_t isr = rtl_inw(rtl, ISR);
		if(isr & 0x20)
		{
			rtl_outw(rtl, ISR, 0x20);
			printf("isr %x\n",isr);
			break;
		}*/
	}
	
	



}
void rtl8139_getmac(struct rtl8139 *rtl, char *mac)
{
	for(int i = 0; i < 6; i++)
	{
		mac[i] = rtl_inb(rtl, i);
	}


}
struct network_dev * rtl8139_init()
{
	struct network_dev *device = kmalloc(sizeof(*device));
	struct rtl8139 *rtl = (struct rtl8139 *)kmalloc(sizeof(*rtl));
	global = rtl;
	device->device = rtl;
	rtl->dev = device;
	rtl->pci = pci_get_device(RTL8139_VEND, RTL8139_DEV);
	rtl->rx_buffer = pallocn(2);//kmalloc(RX_BUF_LEN + RX_BUF_PAD);
	rtl->tx_buffers = (void *)P2V(0x3380000);//kmalloc((8192+16+1500)*4);
	if(rtl->pci != NULL)
	{
		rtl->pci_hdr = rtl->pci->header;
		
		rtl->io_base = pci_get_bar(rtl->pci, PCI_BAR_IO) & ~1;
		
		rtl->mem_base = (uint8_t *)(pci_get_bar(rtl->pci, PCI_BAR_MEM) & ~3);
		
		rtl8139_start(rtl);
		rtl8139_getmac(rtl,(char *)&device->mac);
		device->send = rtl8139_send;
		printf("Realtek 8139 Ethernet adapter Rev %i found\t", rtl->pci_hdr->rev);
		printf("io base %x ",rtl->io_base);
		printf("mem base %x\n",rtl->mem_base);
	//	device->receive = rtl8139_receive;
		print_mac((char *)&device->mac);
		printf("%X\n",rtl->dev);

	}
	else
	{
		kfree(rtl->rx_buffer);
		kfree(rtl);

		rtl = NULL;
		device = NULL;
	}

	return device;

}

