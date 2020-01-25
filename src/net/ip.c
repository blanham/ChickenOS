#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
#include <chicken/net/net_core.h>

//taken from www.netfor2.com/ipsum.htm
uint16_t cksum(uint16_t *buf, size_t nbytes)
{
	int sum, oddbyte;
	const unsigned short *ptr = buf;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		((unsigned char *) & oddbyte)[0] = *(unsigned char *) ptr;
		((unsigned char *) & oddbyte)[1] = 0;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (~sum);
}

struct ip_route_entry {
	uint32_t network;
	uint32_t gateway;
	uint32_t mask;
	LIST_ENTRY(ip_route_entry) elem;
};

LIST_HEAD(, ip_route_entry) ip_route_table = LIST_HEAD_INITIALIZER(ip_route_table);

void ip_route_insert(uint32_t ip, uint32_t gateway, uint32_t mask)
{
	struct ip_route_entry *new = kcalloc(sizeof(*new), 1);
	new->network = ip & mask;
	new->mask = mask;
	new->gateway = gateway;
	LIST_INSERT_HEAD(&ip_route_table, new, elem);
}

uint32_t ip_route_lookup(uint32_t ip)
{
	struct ip_route_entry *iter;
	uint32_t ret = ip;
	LIST_FOREACH(iter, &ip_route_table, elem)
	{
		if((iter->mask & ip) == iter->network)
			ret = iter->gateway;


	}
	return ret; 
}
/*
size_t ip_send(struct network_dev *dev, uint8_t proto,uint8_t *payload, size_t len)
{
	struct ip *ip = kmalloc(sizeof(*ip) + len);
	uint8_t *pay = (uint8_t *)ip + sizeof(*ip);
	uint16_t ethertype = 0x800;
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(len + ip->ip_hl*4);
	ip->ip_p = proto;
	//memcpy(&ip->ip_src, our_ip, 4);
	char dest_mac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	ip->ip_sum = cksum((uint16_t*)ip, ip->ip_hl*4);
	memcpy(pay, payload, len);

	ethernet_send(dev, (uint8_t *)ip, len+sizeof(*ip), ethertype, dest_mac);
	kfree(ip);	
	return len;
}
*/
void ip_send2(struct sockbuf *sb)
{
	
	struct ip *ip = kcalloc(sizeof(*ip) + sb->t_len, 1);
	uint8_t *pay = (uint8_t *)ip + sizeof(*ip);
	uint16_t len = sb->t_len;
	
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(len + ip->ip_hl*4);
	ip->ip_p = sb->t_proto;
	ip->ip_ttl = 64;
	ip->ip_off = 0x40;//should not fragment

	ip->ip_src.s_addr = sb->dev->ip;
	ip->ip_dst.s_addr = htonl(sb->dest_ip);			
	ip->ip_sum = cksum((uint16_t*)ip, ip->ip_hl*4);

	if(sb->broadcast == 0)
	{
		if(sb->dev->resolved == 0)
			return;
	
		if(arp_lookup(sb, ip_route_lookup(sb->dest_ip)) < 0) //->dev, sb->dest_ip, sb->dest_mac);
		{	
			goto send_cleanup;
		}
	}else{
		memset(sb->dest_mac, 0xff, 6);	
	}

	memcpy(pay, sb->transport, len);
	
	sb->i_len = len + sizeof(*ip);
	sb->data = ip;
	sb->length = sb->i_len;

	ethernet_send(sb, ETHERTYPE_IP);
send_cleanup:
	kfree(ip);
	return;

}
void ip_print(struct ip *ip)
{
	printf("ver %x hl %x len %x p %x\n",ip->ip_v,ip->ip_hl,ntohs(ip->ip_len),ip->ip_p);
}
void ip_address_print(uint32_t ip)
{
	uint8_t *p;
	p =(uint8_t *)&ip;
	printf("%i.%i.%i.%i",p[0],p[1],p[2],p[3]);
}
//this might be the place to queue packets if they are not
//icmp packets
void ip_received(struct sockbuf *sb)
{
	struct ip *ip = sb->ip;
	sb->transport = ip + 1;
	sb->t_len = ntohs(ip->ip_len) - sizeof(*ip); 
	switch(ip->ip_p)
	{
		case 0x1:
		icmp_received(sb);
		break;
		case 0x6:
		tcp_received(sb);
		break;
		case 0x11:
		udp_received(sb);
		break;

		default:
		ip_print(ip);

	}
	sockbuf_unqueue(sb);
	sockbuf_free(sb);

}
