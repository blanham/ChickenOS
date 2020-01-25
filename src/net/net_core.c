#include <stdio.h>
#include <stdlib.h>
#include <queue.h>
#include <chicken/common.h>
#include <chicken/device/net/rtl8139.h>
#include <chicken/device/net/pcnet.h>
#include <chicken/device/net/e1000.h>
#include <chicken/net/dhcp.h>
#include <chicken/net/net_core.h>

uint8_t our_ip[] = {128,135,155,203};

uint8_t their_ip[] = {128,135,155,44};
uint8_t dest_mac[] = { 0x00,0x0c,0x29,0x95,0x05,0x2a};
//TAILQ_HEAD(, sockbuf) sockbuf_list = TAILQ_HEAD_INITIALIZER(sockbuf_list);

uint16_t htons(uint16_t val)
{
	uint16_t temp;
	temp = val << 8 | val >> 8;
	return temp;

}
uint16_t ntohs(uint16_t val)
{
	uint16_t temp;
	temp = val << 8 | val >> 8;
	return temp;

}
uint32_t htonl(uint32_t val)
{
uint32_t tmp;
uint8_t *s = (uint8_t*)&val;
uint8_t *d = (uint8_t*)&tmp;	
d[0] = s[3];
d[1] = s[2];
d[2] = s[1];
d[3] = s[0];
return tmp;
}
uint32_t ntohl(uint32_t val)
{
uint32_t tmp;
uint8_t *s = (uint8_t*)&val;
uint8_t *d = (uint8_t*)&tmp;
d[0] = s[3];
d[1] = s[2];
d[2] = s[1];
d[3] = s[0];
return tmp;
}
void print_mac(char *mac)
{

	printf("MAC ADDR: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
		mac[0]&0xff,mac[1]&0xff,mac[2]&0xff,
		mac[3]&0xff,mac[4]&0xff,mac[5]&0xff);

}
struct network_dev *net_device;
char packet2[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xc0, 
0x9f, 0x50, 0x32, 0xa0, 0x08, 0x06, 0x00, 0x01, 
0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0xc0, 
0x9f, 0x50, 0x32, 0xa0, 0x80, 0x87, 0x9b, 0xcb, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x87, 
0x9b, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00 };
char *look = "www.google.com";
int sys_network_setup()
{
	net_device->ip = 0;
	dhcp_getip(net_device);
	while(net_device->ip == 0);
	net_device->resolved = 1;
	printf("Assigned ip: ");
 	ip_address_print(net_device->ip);
	printf(" DNS Server: ");
 	ip_address_print(net_device->dns_ip);
	printf("\n");
	printf("Router: ");
 	ip_address_print(net_device->router[0]);
	printf(" Subnet mask: ");
 	ip_address_print(net_device->subnet_mask);
	printf("\n");
	 ip_route_insert(net_device->ip, net_device->router[0], net_device->subnet_mask);

	//for(int i = 0; i < 20; i++)
	dns_lookup(net_device, "www.google.com");
	return 0;
}
extern void arp_init();
void network_init()
{
//	TAILQ_INIT(&sockbuf_list);
	arp_init();
	//TODO: really need to set up a proper probe of net cards
	//      maybe have a menu to pick if multiple cards are detected?
	net_device = rtl8139_init();
	if(net_device == NULL)
		net_device = e1000_init();
	if(net_device == NULL)
		net_device = pcnet_init();
	net_device->ip = 0;//0xFFFFFFFF;
	net_device->resolved = 0;
}
void network_dev_setip(struct network_dev *dev, uint32_t ip)
{
	dev->ip = ip;
}
void sockbuf_send(struct sockbuf *sb UNUSED)
{


}
struct sockbuf *sockbuf_alloc(struct network_dev *dev, uint32_t len)
{
	struct sockbuf *new = kcalloc(sizeof(*new), 1);// + 256);
	new->data = kcalloc(len, 1);
	new->dev = dev;
	new->length = len;
	sockbuf_queue(new);
	return new;
}
void sockbuf_free(struct sockbuf *sb UNUSED)
{
//	kfree(sb->data);
//	kfree(sb);
}
void sockbuf_queue(struct sockbuf *sb UNUSED)
{
//	TAILQ_INSERT_HEAD(&sockbuf_list, sb, elem);
}
void sockbuf_unqueue(struct sockbuf *sb UNUSED)
{
//	TAILQ_REMOVE(&sockbuf_list, sb, elem);
}
void network_received(struct sockbuf *sb)
{
	ethernet_received(sb);
}
