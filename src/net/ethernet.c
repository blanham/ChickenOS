#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>
#include <string.h>
void ethernet_printmac(char *name, uint8_t *mac)
{
	if(name != NULL)
	printf("%s ", name);
	for(int i = 0; i < ETH_ALEN; i++)
		printf("%x:",mac[i] & 0xff);
	printf("\n");


}
void ethernet_print(struct ether_header *eth)
{
	ethernet_printmac("dhost", eth->ether_dhost);
	ethernet_printmac("shost", eth->ether_shost);
	printf("ether_type %x\n",ntohs(eth->ether_type));
}
int ememcmp(const void *Ptr1, const void *Ptr2, size_t Count)
{
    int v = 0;
    char *p1 = (char *)Ptr1;
    char *p2 = (char *)Ptr2;

    while(Count-- > 0 && v == 0) {
        v = *(p1++) - *(p2++);
    }

    return v;
}
void ethernet_received(struct sockbuf *sb)
{
	struct ether_header *eth = sb->data;
	uint16_t ether_type = htons(eth->ether_type);
	void *inet = sb->data + sizeof(*eth);
	//printf("ether\n");
	char tmp[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	if(ether_type == ETHERTYPE_ARP)
	{
	//	printf("arp\n");
	}
	if(!ememcmp(eth->ether_dhost, tmp, 6))
	{
		if(ether_type != ETHERTYPE_ARP)
			goto error;
		else{//printf("ARP\n");
		goto ARP;}
	}
	if(ememcmp(eth->ether_dhost, sb->dev->mac, 6))
	{
			goto error;
	}
ARP:
	switch(ether_type)
	{
		case ETHERTYPE_ARP:
		sb->arp = inet;
		arp_received(sb);
		break;
		case ETHERTYPE_IP:
		sb->ip = inet;
		ip_received(sb);
		break;
		default:
			
	//	printf("%x\n",ether_type);
		sockbuf_unqueue(sb);
		sockbuf_free(sb);

	}
	return;

error:
	sockbuf_unqueue(sb);
	sockbuf_free(sb);

}
void ethernet_send2(struct network_dev *dev, uint8_t *payload, size_t len, uint16_t ethertype, char dest_mac[6])
{
	
	struct ether_header *eth = kmalloc(len + sizeof(*eth));
	uint8_t *pay = (uint8_t *)eth + sizeof(*eth);
	kmemcpy(pay, payload, len);
	eth->ether_type = htons(ethertype);
	kmemcpy(eth->ether_dhost, dest_mac, 6);
	kmemcpy(eth->ether_shost, dev->mac, 6);
//	ethernet_print(eth);

	dev->send(dev, (uint8_t *)eth, len + sizeof(*eth));
//	for(int i = 0; i < 60; i++)
	//	{
	//		printf("%X:",+ i);
	//		if(i %8 == 7)
	//			printf("\n");
	//	}
	kfree(eth);

}

void ethernet_send(struct sockbuf *sb, uint16_t ether_type)
{
	ethernet_send2(sb->dev, sb->data,sb->length, ether_type, (char *)sb->dest_mac); 
}


