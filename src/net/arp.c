#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
#include <chicken/net/net_core.h>
//use stupid timeout for now
//since i am too lazy to implement
//a proper clock as of yet
struct arp_cache_entry {
	struct  ether_arp arp;
	int timeout;
	uint32_t ip;
	char mac[6];	
	LIST_ENTRY(arp_cache_entry) elem;
};

LIST_HEAD(test , arp_cache_entry) arp_cache = LIST_HEAD_INITIALIZER(arp_cache);

struct arp_queue_entry {
	struct sockbuf *sb;
	uint32_t ip;	
	LIST_ENTRY(arp_queue_entry) elem;
};

LIST_HEAD(test2, arp_queue_entry) arp_queue = LIST_HEAD_INITIALIZER(arp_queue);

extern void print_ip(uint32_t ip);
void arp_init()
{
	LIST_INIT(&arp_queue);


}
void arp_send(struct network_dev *dev, uint32_t ip)
{
	struct ether_arp *arp;
	struct sockbuf *sb;
	arp = kcalloc(sizeof(*arp) , 1);
	//ip = (ip);
	arp->type = htons(1);
	arp->proto = htons(0x800);
	arp->hlen = 6;
	arp->plen = 4;
	arp->op = htons(1);
	memcpy(arp->arp_sha,dev->mac, 6);
	memset(arp->arp_tha,0, 6);
	memcpy(arp->arp_spa,&dev->ip, 4);
	memcpy(arp->arp_tpa,&ip, 4);

	sb = sockbuf_alloc(dev, sizeof(*arp));
	sb->arp = kmalloc(sizeof(*arp));
	memcpy(sb->arp, arp, sizeof(*arp));	
	memcpy(sb->data, arp, sizeof(*arp));	
	memset(sb->dest_mac, 0xff, 6);
	sb->length = 42;
	ethernet_send(sb, ETHERTYPE_ARP);
	sockbuf_free(sb);




}
extern int ememcmp(const void *Ptr1, const void *Ptr2, size_t Count);
void arp_received_request(struct sockbuf *sb)
{
	struct arp_cache_entry *iter;
	struct sockbuf *new_sb;
	struct ether_arp *arp = sb->arp;
	struct arp_cache_entry *entry;
	struct network_dev *dev = sb->dev;
	if(dev == NULL)
		printf("error in %s\n", __func__);
	if(dev->resolved == 0) //no ip, therefore we can't send a reply
	{	
		goto cleanup;	
	}
	//print_ip(*(uint32_t *)arp->arp_tpa);
	//print_ip(dev->ip);
	if(!ememcmp(arp->arp_tpa, (uint8_t *)&dev->ip, 4))
	{
		entry = kmalloc(sizeof(*entry) + 64);
		memcpy(&entry->arp, arp, sizeof(*arp));
		entry->arp.op = htons(2);
		memcpy(entry->arp.arp_sha,dev->mac, 6);
		memcpy(entry->arp.arp_tha,arp->arp_sha, 6);
		memcpy(entry->arp.arp_spa,arp->arp_tpa, 4);
		memcpy(entry->arp.arp_tpa,arp->arp_spa, 4);
		entry->timeout = 0;
		new_sb = sockbuf_alloc(dev, sizeof(*arp));
		new_sb->arp = kmalloc(sizeof(*arp));
		memcpy(new_sb->arp, &entry->arp, sizeof(*arp));	
		memcpy(new_sb->data, &entry->arp, sizeof(*arp));	
		memcpy(new_sb->dest_mac, arp->arp_sha, 6);
		ethernet_send(new_sb, ETHERTYPE_ARP);
		sockbuf_free(new_sb);
		memcpy(entry->mac, arp->arp_sha, 6);
		memcpy(&entry->ip, arp->arp_spa, 4);
		LIST_INSERT_HEAD(&arp_cache, entry, elem);
	}
	LIST_FOREACH(iter, &arp_cache, elem)
	{
		iter->timeout++;
			

	}
cleanup:
	sockbuf_unqueue(sb);
	sockbuf_free(sb);
}

void arp_received_reply(struct sockbuf *sb)
{
	struct arp_cache_entry *iter;
	struct arp_queue_entry *que, *tmp;
	struct ether_arp *arp = sb->arp;
	struct arp_cache_entry *entry;
	
	entry = kmalloc(sizeof(*entry) + 64);
	memcpy(&entry->arp, arp, sizeof(*arp));
	entry->arp.op = htons(2);
	memcpy(entry->arp.arp_sha,sb->dev->mac, 6);
	memcpy(entry->arp.arp_tha,arp->arp_sha, 6);
	memcpy(entry->arp.arp_spa,arp->arp_tpa, 4);
	memcpy(entry->arp.arp_tpa,arp->arp_spa, 4);
	entry->timeout = 0;
	memcpy(entry->mac, arp->arp_sha, 6);
	memcpy(&entry->ip, arp->arp_spa, 4);
	LIST_INSERT_HEAD(&arp_cache, entry, elem);
	
	
	LIST_FOREACH(iter, &arp_cache, elem)
	{
		iter->timeout++;
	}
	
	LIST_FOREACH_SAFE(que, &arp_queue, elem, tmp)
	{
		if(entry->ip == que->ip)
		{
			LIST_REMOVE(que, elem);
			ip_send2(que->sb);
			kfree(que);
		}	
	}
}
void arp_received(struct sockbuf *sb)
{
	struct ether_arp *arp = sb->arp;
	uint16_t op = ntohs(arp->op);


	switch(op)
	{
		case 0x1://request
		arp_received_request(sb);
		break;
		case 0x2:
		arp_received_reply(sb);
		break;
		default:
		printf("arp %x\n",op);

	}


}
int arp_lookup(struct sockbuf *sb, uint32_t ip)//struct network_dev *dev, uint32_t ip, char *mac)
{
	struct arp_cache_entry *iter;
	struct arp_queue_entry *que;	
	uint32_t n_ip = htonl(ip);
	
	LIST_FOREACH(iter, &arp_cache, elem)
	{
		if(iter->ip == n_ip)
		{
			memcpy(sb->dest_mac, iter->mac, 6);
			return 0;
		}	

	}
	
	if(sb->dev->resolved == 0) //no ip, therefore we can't send a reply
		return -1;
	
	arp_send(sb->dev, n_ip);
	que = kcalloc(sizeof(*que),1);
	que->ip = n_ip;
	que->sb = sb;
	LIST_INSERT_HEAD(&arp_queue, que, elem);	
	return -1;

}

