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
void rtl_handler(struct registers *regs)
{
	regs = regs;
	struct rtl8139 *rtl = global;
	uint16_t irq = rtl_inw(global, ISR);
	if(irq & 0x20){
		rtl_outw(rtl, ISR, 0x20);
	}
	if(irq & 0x10){

		rtl_outw(rtl, ISR, 0x10);
	}
	if(irq & 0x4){
		rtl_outw(rtl, ISR, 0x4);
		printf("tx\n");
	}

	if(irq & 0x1)
	{	
		for(int i = 0; i < 0x100; i++)
		{
	//		printf("%c:",*(char *)(rtl->rcv_buffer + i) & 0xff) ;
		//	if(i % 32 == 31)
		//		printf("\n");
		}

	//	printf("received packet!\n");
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

void testsend_packet()//truct rtl8139 *rtl)
{

}

size_t rtl8139_send(struct network_dev *dev, uint8_t *_buf, size_t length)
{
	struct rtl8139 *rtl = dev->device;
	rtl = rtl;
	_buf = _buf;
	length = length;
	
	void* tx_buffer = (void *)(rtl->tx_buffers + 8192*rtl->tx_cur);
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
	
	kmemset(rtl->rcv_buffer, 0, (8192*8)+16+1500);
	rtl_outl(rtl, 0x30,(uintptr_t)V2P(rtl->rcv_buffer));
	rtl_outb(rtl, 0x37, 0xc);
	
	for(int i=0; i < 4; i++)
	{
		rtl_outl(rtl, 0x20 + i*4, (uintptr_t)V2P(rtl->tx_buffers) + i*(8192 +16+1500));
	}
	//TODO: need to register pci IRQs instead of doing it directly
	interrupt_register(32 + rtl->pci_hdr->int_line, &rtl_handler);
	rtl_outl(rtl, 0x44, (1 << 7) |  (1 << 1));
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
	rtl->pci = pci_get_device(RTL8139_VEND, RTL8139_DEV);
	rtl->rcv_buffer = kmalloc((8192*8)+16+1500);
	rtl->tx_buffers = (void *)P2V(0x3380000);//kmalloc((8192+16+1500)*4);
	if(rtl->pci != NULL)
	{
		rtl->pci_hdr = &rtl->pci->header.hdr;
		printf("Realtek 8139 Ethernet adapter Rev %i found\n", rtl->pci_hdr->rev);
		
		rtl->io_base = pci_get_bar(rtl->pci, PCI_BAR_IO) & ~1;
		printf("io base %x\n",rtl->io_base);
		
		rtl->mem_base = (uint8_t *)(pci_get_bar(rtl->pci, PCI_BAR_MEM) & ~3);
		printf("mem base %x\n",rtl->mem_base);
		
		rtl8139_start(rtl);
		rtl8139_getmac(rtl,(char *)&device->mac);
		device->send = rtl8139_send;
	//	device->receive = rtl8139_receive;

	}
	else
	{
		kfree(rtl->rcv_buffer);
		kfree(rtl);

		rtl = NULL;
		device = NULL;
	}

	return device;

}

