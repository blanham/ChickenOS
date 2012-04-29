#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>


struct udp {
	uint16_t src;
	uint16_t dst;
	uint16_t length;
	uint16_t checksum;
};

void udp_send(struct network_dev *dev, uint16_t src, uint16_t dst, 
	uint8_t *payload, size_t len)
{
	struct udp *udp = kmalloc(sizeof(*udp) + len);
	uint8_t *pay = (uint8_t *)udp + sizeof(*udp);
	udp->src = htons(src);
	udp->dst = htons(dst);
	udp->length = htons(len + sizeof(*udp));
	kmemcpy(pay, payload, len);
	printf("length %i\n",udp->length);
	ip_send(dev, 17, (uint8_t *)udp, len + sizeof(*udp));
	kfree(udp);
}
