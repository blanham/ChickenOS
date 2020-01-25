#include <stdio.h>
#include <string.h>
#include <chicken/net/net_core.h>

struct icmp {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t rest;	
} __attribute__((packed));
/*struct icmp {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t rest;	
} __attribute__((packed));*/

void icmp_echoreceived(struct sockbuf *rsb)
{
	struct sockbuf *sb;
	size_t len;
	struct icmp *icmp = rsb->transport;
	len = rsb->t_len;
//	len = sizeof(struct icmp);
//	icmp = kmalloc(len);
//	memset(icmp, 0, len);	
	icmp->type = 0;
	icmp->code = 0;
	icmp->checksum = 0;
	uint16_t sum = cksum((uint16_t *)icmp, len);
	icmp->checksum = sum;
	sb = sockbuf_alloc(rsb->dev, len);
	sb->transport = icmp;
	sb->t_len = len;
	sb->t_proto = 0x1;
	sb->dest_ip = htonl(rsb->ip->ip_src.s_addr);
	memcpy(sb->dest_mac, ((struct ether_header *)rsb->data)->ether_shost ,6);
	ip_send2(sb);
	//kfree(icmp);
	sockbuf_free(sb);

}

void icmp_received(struct sockbuf *sb)
{
	struct icmp *icmp = sb->transport;
	uint8_t type = icmp->type;
	switch(type)
	{
		case 0x8:
		icmp_echoreceived(sb);
		break;
		default:
			;
	}
}