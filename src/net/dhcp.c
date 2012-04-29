#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>


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

void dhcp_send(struct network_dev *dev)
{
	struct dhcp_message *dhcp = kmalloc(sizeof(*dhcp));
	uint16_t src, dst;
	src = 0;
	dst = 0;
	dhcp->op = 0;
	dhcp->htype = 1;



	udp_send(dev, src, dst, (uint8_t *)dhcp, sizeof(*dhcp));

	kfree(dhcp);
}

