#include <stdio.h>
#include <string.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>

struct dns {
	uint16_t id;
	unsigned qr:1;
	unsigned op:4;
	unsigned aa:1;
	unsigned tc:1;
	unsigned rd:1;
	unsigned ra:1;
	unsigned z:1;
	unsigned ad:1;
	unsigned cd:1;
	unsigned rcode:4;
	uint16_t num_ques;
	uint16_t total_ans;
	uint16_t total_auth;
	uint16_t total_add;
} __attribute__((packed));
struct dns_query {
	uint16_t type;
	uint16_t class;
};
struct dns_query dns_query_ip4;
#define DNSQR_QUERY 	0
#define DNSAR_RESPONSE 	1
#define DNSOP_QUERY 	0

char *dns_breakstr(char *name)
{
	char *p = name;
	char *out =kcalloc(256*5,1);
	char *ret = out;
	uint8_t cnt = 0;
	char *save;
	p = strtok_r (p, ".", &save);
	while(p != NULL)
	{
		cnt = strlen(p);
		*out++ = cnt;
		kmemcpy(out, p, cnt);
		out += cnt;

		p = strtok_r (NULL, ".", &save);
	}
	*out++ = 0;
//	printf("%s\n",ret);
	return ret;
}
void dns_sendquery(struct network_dev *dev, char * name)
{
	struct dns *dns = kcalloc(sizeof(*dns) + 256*4, 1);
	printf("looking up %s\n",name);
	void *queries = dns + 1;
	struct sockbuf *sb;
	struct dns_query *foot;
	size_t len = sizeof(*dns);
	char *dom;	
	dns->id = 0x4242;
	dns->qr = DNSQR_QUERY;
	dns->op = DNSOP_QUERY;
	dns->num_ques = htons(1);
	dom = dns_breakstr(name);
	len += strlen(dom) + 1;
	kmemcpy(queries, dom, strlen(dom) + 1);
	foot = (void *)((uint8_t*)dns + len);
	foot->type = htons(1);
	foot->class = htons(1);
	len+=sizeof(*foot);	
	sb = sockbuf_alloc(dev, len);
	sb->payload = dns;
	sb->broadcast = 0;
	sb->dest_ip = htonl(dev->dns_ip);
	sb->p_len = len;
	
	udp_send2(sb, 23232, 53);	
//	kfree(dns);
//	sockbuf_free(sb);
}

void dns_lookup(struct network_dev *dev, char *domain)
{
	
	uint32_t bound =udp_bind(dev, 53);
	(void)bound;
	dns_sendquery(dev, domain);



}
