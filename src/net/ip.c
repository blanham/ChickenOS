#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>


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


size_t ip_send(struct network_dev *dev, uint8_t proto,uint8_t *payload, size_t len)
{
	struct ip *ip = kmalloc(sizeof(*ip) + len);
	uint8_t *pay = (uint8_t *)ip + sizeof(*ip);
	uint16_t ethertype = 0x800;
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_len = htons(len + ip->ip_hl*4);
	ip->ip_p = proto;
	kmemcpy(&ip->ip_src, our_ip, 4);
	char dest_mac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	ip->ip_sum = cksum((uint16_t*)ip, ip->ip_hl*4);
	kmemcpy(pay, payload, len);

	ethernet_send(dev, (uint8_t *)ip, len+sizeof(*ip), ethertype, dest_mac);
	kfree(ip);	
	return len;
}
