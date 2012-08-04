#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>
#include <net/dhcp.h>
//#define DHCP_DEBUG
#ifdef DHCP_DEBUG
	#define DPRINTF(fmt, ...) printf("DHCP:" fmt,  ## __VA_ARGS__);
#else
	#define DPRINTF(fmt, ...) 
#endif
struct dhcp_message {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
};
uint8_t magic[] = {0x63, 0x82, 0x53, 0x63};
uint8_t hardcoded_ip[]= {128,135,155,49};
uint32_t dhcp_timeout;// = 0x15180;

#define DHCP_OP_REQUEST 1
#define DHCP_OP_REPLY   2
#define DHCP_TYPE_DISCOVER 0x1
#define DHCP_TYPE_OFFER 0x2
#define DHCP_TYPE_REQUEST 0x3
#define DHCP_TYPE_ACK 	0x5
int  dhcp_getoption(struct dhcp_message *dhcp, uint8_t msg, void *out, size_t len)
{
	uint8_t *options = ((uint8_t *)dhcp) + sizeof(*dhcp)+4 ;
	int found = 0;
	while(!found)
	{
		if(*options == msg)
		{
			found = 1;
			break;	
		}else if(*options == 0xff){
			return -1;	
		}
		options += options[1] + 2;
	}	
	if(len == 0)
	len = options[1];
	kmemcpy(out, options  +2, len);
	return 0;
}
void dhcp_populateheader(struct network_dev *dev, struct dhcp_message *dhcp, uint8_t **options, uint8_t op, uint8_t msg)
{
	dhcp->op = op;
	dhcp->htype = 1;
	dhcp->hlen = 6;
	kmemcpy(&dhcp->xid,&dev->mac[2], 4);
	kmemcpy(dhcp->chaddr, dev->mac, 6);
	kmemcpy(*options, magic, 4);
	*options+=4;
	
	*(*options)++ = 0x35;
	*(*options)++ = 0x01;
	*(*options)++ = msg;
}
void dhcp_endheader(uint8_t **options)
{
	*(*options)++ = 0xff;
}

void dhcp_send(struct network_dev *dev, struct dhcp_message *dhcp, size_t len)
{
	struct sockbuf *sb = sockbuf_alloc(dev, len);
	sb->payload = dhcp;
	sb->broadcast = 1;
	sb->dest_ip = 0xffffffff;
	sb->p_len = len;
	udp_send2(sb, 68, 67);
	sockbuf_free(sb);

}

void dhcp_release(struct network_dev *dev)
{
	struct dhcp_message *dhcp = kmalloc(sizeof(*dhcp) + 256);
	uint8_t *options = ((uint8_t *)dhcp) + sizeof(*dhcp);
	

	dhcp_populateheader(dev, dhcp, &options, DHCP_OP_REQUEST, 7);
	
	//ask for certain ip address
	*options++ = 0x32;
	*options++ = 0x04;
	kmemcpy(options, &dev->ip, 4);
	options+=4;
	*options++ = 0x3d;
	*options++ = 0x07;
	*options++ = 0x01;
	kmemcpy(options, (void *)dev->mac, 6);
	options +=6;

	dhcp_endheader(&options);
	
	dhcp_send(dev, dhcp, (uintptr_t)options - (uintptr_t)dhcp);
	kfree(dhcp);}

void dhcp_senddiscover(struct network_dev *dev)
{
	struct dhcp_message *dhcp = kcalloc(sizeof(*dhcp) + 256, 1);
	uint8_t *options = ((uint8_t *)dhcp) + sizeof(*dhcp);
	
	dhcp_populateheader(dev, dhcp, &options, DHCP_OP_REQUEST, DHCP_TYPE_DISCOVER);
	dhcp_endheader(&options);
	dhcp_send(dev, dhcp, (uintptr_t)options - (uintptr_t)dhcp);
	kfree(dhcp);
}

void dhcp_sendrequest(struct sockbuf *rsb)
{
	
	struct dhcp_message *rcv = rsb->payload;
	struct dhcp_message *dhcp = kcalloc(sizeof(*dhcp) + 256, 1);
	uint8_t *options = ((uint8_t *)dhcp) + sizeof(*dhcp);
	
	
	dhcp_populateheader(rsb->dev, dhcp, &options, DHCP_OP_REQUEST, DHCP_TYPE_REQUEST);
	
	//parameter request list
	*options++ = 0x37;
	*options++ = 0x03;
	*options++ = 0x01;//Subnet mask
	*options++ = 0x03;//Router
	*options++ = 0x06;//DNS
	
	//request ip address
	*options++ = 0x32;
	*options++ = 0x04;
	*((uint32_t *)options)  = rcv->yiaddr;
	options+=4;

	//DHCP server that sent offer
	*options++ = 0x36;
	*options++ = 0x04;
	dhcp_getoption(rcv, 54, options, 4);
	options+=4;
	
	dhcp_endheader(&options);
	
	dhcp_send(rsb->dev, dhcp, (uintptr_t)options - (uintptr_t)dhcp);
	kfree(dhcp);
}
int dhcp_type(struct dhcp_message *dhcp)
{
	uint8_t *options = ((uint8_t *)dhcp) + sizeof(*dhcp);
	while(*options != 53)
		options++;
	options+=2;
	return *options;

}
void print_ip(uint32_t ip)
{
	uint8_t *p;
	p =(uint8_t *)&ip;
	printf("%i.%i.%i.%i\n",p[0],p[1],p[2],p[3]);
}
void dhcp_acked(struct sockbuf *sb)
{
	struct dhcp_message *dhcp = sb->payload;
	struct network_dev *dev = sb->dev;
	if(dev->ip != 0)
		return;
	dev->ip = dhcp->yiaddr;
	dhcp_getoption(dhcp, 1, (uint8_t *)&dev->subnet_mask, 4);
	dhcp_getoption(dhcp, 6, (uint8_t *)&dev->dns_ip, 4);
	dhcp_getoption(dhcp, 3, (uint8_t *)&dev->router, 8);
	DPRINTF("assigned ip:");
	#ifdef DHCP_DEBUG
	print_ip(dev->ip);
	#endif

}
void dhcp_received(struct sockbuf *sb)
{
	struct dhcp_message *dhcp = sb->payload;
	DPRINTF("received dhcp\n");
	uint8_t op = dhcp->op;
	DPRINTF("op %x\n",op);
	uint32_t type = dhcp_type(dhcp);
	dhcp_getoption(dhcp, 53, &type, 1);
	DPRINTF("type %x\n",type);
	if(op == DHCP_OP_REPLY)
	{
		switch(type)
		{
			case DHCP_TYPE_OFFER:
			dhcp_sendrequest(sb);	
			break;
			case DHCP_TYPE_ACK:
			dhcp_acked(sb);
			break;
			default:
			break;
		}
	}
}
void dhcp_getip(struct network_dev *dev)
{
	DPRINTF("requesting ip address\n");
	dhcp_timeout = htonl(0x15180);
	dhcp_timeout = htonl(0x180);
//	dhcp_release(dev);
	dhcp_senddiscover(dev);
}

