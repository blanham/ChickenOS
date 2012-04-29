#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>


void ethernet_send(struct network_dev *dev, uint8_t *payload, size_t len, uint16_t ethertype, char dest_mac[6])
{
	
	struct ether_header *eth = kmalloc(len + sizeof(*eth));
	uint8_t *pay = (uint8_t *)eth + sizeof(*eth);
	kmemcpy(pay, payload, len);
	eth->ether_type = htons(ethertype);
	kmemcpy(eth->ether_dhost, dest_mac, 6);
	kmemcpy(eth->ether_shost, dev->mac, 6);
	dev->send(dev, (uint8_t *)eth, len + sizeof(*eth));
//	for(int i = 0; i < 60; i++)
	//	{
	//		printf("%X:",+ i);
	//		if(i %8 == 7)
	//			printf("\n");
	//	}
	kfree(eth);

}
