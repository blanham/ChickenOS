#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>
enum tcp_states {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2,CLOSE_WAIT, CLOSING, LAST_ACK,TIME_WAIT, BREAK};

struct tcp_state {
	enum tcp_states status;


};
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_PSH 0x08
#define TCP_ACK 0x10
struct tcp {
	uint16_t src;
	uint16_t dst;
	uint32_t seq;
	uint32_t acknum;
	// uint16_t herp;
	unsigned p:4;
	unsigned offset:4;
	
	// uint8_t offset;
	/* int offset:4;
	int res:3;
	int ns:1;*/
	/* int cwr:1;
	int ece:1;
	int urg:1;
	int ack:1;
	int psh:1;
	int rst:1;
	int syn:1;
	int fin:1;*/
	uint8_t flags;
	uint16_t window;
	uint16_t cksum;
	uint16_t urgent;
} __attribute__((packed));
struct tcp_tcb {
	enum tcp_states state;


};
struct tcp_tcb *tcp_alloctcb()
{
	struct tcp_tcb *new;
	new = kcalloc(1, sizeof(*new));
	new->state = CLOSED;
	return new;
}
void tcp_print(struct tcp *hdr)
{
printf("src %i dst %i seq %u acknum %u\n",ntohs(hdr->src),ntohs(hdr->dst),
ntohl(hdr->seq),ntohl(hdr->acknum));
/* printf("offset %u res %u ns %u cwr %u ece %u urg %u ack%u psh %u rst %u syn %u fin %u\n",
hdr->offset,hdr->res,hdr->ns,hdr->cwr,hdr->ece,
hdr->urg,hdr->ack,hdr->psh,hdr->rst,hdr->syn,hdr->fin);*/
printf("offset %u flags %x\n",	
hdr->offset,hdr->flags);//hdr->cwr,hdr->ece,
printf("window %i cksum %x urgent %x\n",
ntohs(hdr->window), ntohs(hdr->cksum), ntohs(hdr->urgent));
}
void tcp_open()
{


}

void tcp_received(struct sockbuf *sb)
{
	struct tcp *tcp = sb->transport;
	tcp_print(tcp);

}
