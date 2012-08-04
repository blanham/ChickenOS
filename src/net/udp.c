#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>

#ifdef UDP_DEBUG
#define DPRINTF(fmt, ...) 
	printf("UDP:" fmt,  ## __VA_ARGS__);
#else
	#define DPRINTF(fmt, ...) 
#endif



extern void hex_dump(void *ptr, int n);
struct udp {
	uint16_t src;
	uint16_t dst;
	uint16_t length;
	uint16_t checksum;
};
struct udp_bound {
	uint16_t port;
	uint32_t num;
	struct network_dev *dev;
	
	TAILQ_ENTRY(udp_bound) elem;
};

TAILQ_HEAD(, udp_bound) udp_list = TAILQ_HEAD_INITIALIZER(udp_list);
struct udp_cached {
	uint16_t src;
	uint16_t dst;
	uint16_t length;
	void *data;
	struct network_dev *dev;
	
	TAILQ_ENTRY(udp_cached) elem;
};

TAILQ_HEAD(, udp_cached) udp_cache = TAILQ_HEAD_INITIALIZER(udp_cache);

void udp_print(struct udp *udp)
{
	printf("src %i dst %i length %x\n",ntohs(udp->src), ntohs(udp->dst),ntohs(udp->length));
}



uint16_t udp_get_next_ephermeral()
{
	static uint16_t port = 41592;
	uint16_t ret = port;
	port++;
	return ret;


}
void udp_send2(struct sockbuf *sb, uint16_t src, uint16_t dst)
{
	struct udp *udp = kmalloc(sizeof(*udp) + sb->p_len);
	uint8_t *pay = (uint8_t *)udp + sizeof(*udp);
	udp->src = htons(src);
	udp->dst = htons(dst);
	udp->length = htons(sb->length + sizeof(*udp));
	udp->checksum = 0;
	sb->transport = udp;
	kmemcpy(pay, sb->payload, sb->p_len);
	sb->t_len = sizeof(*udp) + sb->p_len;
	sb->t_proto = 0x11;
	ip_send2(sb);
}
void udp_received(struct sockbuf *sb)
{
	struct udp *udp = sb->transport;
	struct udp_bound *iter;
	struct udp_cached *rx;
	uint16_t port = ntohs(udp->dst);
	sb->payload = (uint8_t *)udp + sizeof(*udp);
	switch(port)
	{
		case 68:
		dhcp_received(sb);
		break;
		default:
		udp_print(udp);
		//hex_dump(sb->payload, ntohs(udp->length)/16);	
		break;

	}
	return;
	TAILQ_FOREACH(iter, &udp_list, elem)
	{
		if(iter->port == port)
			goto bound;

	}

//	sockbuf_unqueue(sb);
//	sockbuf_free(sb);
	return;
bound:
	rx = kcalloc(sizeof(*rx), 1);
	rx->dst = ntohs(udp->dst);
	rx->src = ntohs(udp->src);
	rx->length = ntohs(udp->length);
	rx->data = kmalloc(rx->length);
	kmemcpy(rx->data, sb->payload, rx->length);
	TAILQ_INSERT_HEAD(&udp_cache, rx, elem);
	
}
size_t udp_receive(uint32_t num, uint8_t *buf, size_t len)
{
	struct udp_bound *iter;
	struct udp_cached *rx;
	int found = 0;
	size_t rx_len = 0;
	TAILQ_FOREACH(iter, &udp_list, elem)
	{
		if(iter->num == num)
			goto good;

	}
	return -1;
good:
	while(found == 0)
	{
		TAILQ_FOREACH(rx, &udp_cache, elem)
		{
			if(rx->dst == iter->port)
				found = 1;

		}
	}
	TAILQ_REMOVE(&udp_cache, rx, elem);	
	rx_len = rx->length;
	if(rx_len > len)
	{
		kfree(rx->data);
		kfree(rx);
		return -1;
	}
	kmemcpy(buf, rx->data, rx_len);
	kfree(rx->data);
	kfree(rx);
	return rx_len;		

}
uint32_t udp_bind(struct network_dev *dev, uint16_t port)
{
	static uint32_t num = 0;
	struct udp_bound *new = kcalloc(sizeof(*new), 1);
	new->num = num;
	new->port = port;
	new->dev = dev;
	TAILQ_INSERT_HEAD(&udp_list, new, elem);
	num++;
	return num-1;
}
