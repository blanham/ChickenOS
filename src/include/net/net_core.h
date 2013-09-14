#ifndef C_OS_NET_NETCORE_H
#define C_OS_NET_NETCORE_H
#include <types.h>
#include <queue.h>
#define ETH_ALEN 6
#define MY_DEST_MAC0    0x00
#define MY_DEST_MAC1    0xC0
#define MY_DEST_MAC2    0x9F
#define MY_DEST_MAC3    0x50
#define MY_DEST_MAC4    0x32
#define MY_DEST_MAC5    0xA0

struct network_dev;

typedef size_t (*net_send_packet_t) (struct network_dev *, uint8_t *_buf, size_t length);
typedef size_t (*net_receive_packet_t) (struct network_dev *, uint8_t *_buf, size_t length);
struct network_dev {
	void *device;
	net_send_packet_t send;
	net_receive_packet_t receive;
	char mac[6];
	uint32_t ip;
	uint32_t dns_ip;
	uint32_t subnet_mask;
	uint32_t router[2];
	int resolved;
	struct network_dev *next;
};
struct ether_header
{
  uint8_t  ether_dhost[ETH_ALEN];  /* destination eth addr */
  uint8_t  ether_shost[ETH_ALEN];  /* source ether addr    */
  uint16_t ether_type;             /* packet type ID field */
} __attribute__ ((__packed__));

struct  ether_arp {
    uint16_t type;
	uint16_t proto;
	uint8_t hlen;
	uint8_t plen;
	uint16_t op;
	uint8_t arp_sha[ETH_ALEN]; /* sender hardware address */
    uint8_t arp_spa[4];        /* sender protocol address */
    uint8_t arp_tha[ETH_ALEN]; /* target hardware address */
    uint8_t arp_tpa[4];        /* target protocol address */
} __attribute__((packed));
struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
	
};

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct ip {
   uint8_t        ip_hl:4; /* both fields are 4 bytes */
   uint8_t        ip_v:4;
   uint8_t        ip_tos;
   uint16_t       ip_len;
   uint16_t       ip_id;
   uint16_t       ip_off;
   uint8_t        ip_ttl;
   uint8_t        ip_p;
   uint16_t       ip_sum;
   struct in_addr ip_src;
   struct in_addr ip_dst;
};
extern uint8_t our_ip[];
extern uint8_t their_ip[];
extern uint8_t dest_mac[];
uint16_t htons(uint16_t val);
uint16_t ntohs(uint16_t val);
uint32_t htonl(uint32_t val);
uint32_t ntohl(uint32_t val);

enum proto_type {
	IPPROTO_ICMP = 1,
	IPPROTO_TCP = 6,
	IPPROTO_UDP = 17
};



struct socket {
	struct network_dev *dev;
	int pid;
	//enum sock_type type;
	enum proto_type proto;
	uint16_t src_port;
	uint16_t dst_port;	

};
struct addri{
    int     ai_flags;
    int     ai_family;
    int     ai_socktype;
    int     ai_protocol;
    size_t  ai_addrlen;
    struct  sockaddr *ai_addr;
    char    *ai_canonname;     /* canonical name */
    struct  addrinfo *ai_next; /* this struct can form a linked list */
};
enum transport_type {
	CSOCK_TCP,
	CSOCK_UDP,
	CSOCK_RAW,
	CSOCK_ARP
};
enum build_packet {
	CSOCK_PAYLOAD,
	CSOCK_PHYSICAL,
	CSOCK_INTERNET,
	CSOCK_TRANSPORT
};
struct sockbuf {
	size_t length;
	void *data;
	struct ether_header *eth;
	
	struct ip *ip;
	struct ether_arp *arp;
	uint16_t proto;
	uint32_t i_len;
	
	void *transport;
	uint16_t t_proto;
	uint32_t t_len;;
	
	void *payload;
	uint32_t p_len;
	
	struct network_dev * dev;
	struct socket *socket;
	char dest_mac[6];
	uint32_t dest_ip;
	int broadcast;
	
	TAILQ_ENTRY(sockbuf) elem;
};
struct sockbuf *sockbuf_alloc(struct network_dev *dev, uint32_t len);
void sockbuf_free(struct sockbuf *sb);

void sockbuf_queue(struct sockbuf *sb);
void sockbuf_unqueue(struct sockbuf *sb);
int getaddrinfo(const char *node,
                const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
 
void freeaddrinfo(struct addrinfo *res);
int socket(int domain, int type, int protocol); 


void udp_send2(struct sockbuf *sb, uint16_t src, uint16_t dst);
void udp_received(struct sockbuf *sb);
uint32_t udp_bind(struct network_dev *dev, uint16_t port);


size_t ip_send(struct network_dev *dev, uint8_t proto,uint8_t *payload, size_t len);
void ip_send2(struct sockbuf *sb);
void ip_received(struct sockbuf *sb);

void ip_address_print(uint32_t ip);
void ip_route_insert(uint32_t ip, uint32_t gateway, uint32_t mask);

void icmp_received(struct sockbuf *sb);

void tcp_received(struct sockbuf *sb);
void dhcp_received(struct sockbuf *sb);

	
void dns_lookup(struct network_dev *dev, char *domain);

void ethernet_received(struct sockbuf *sb);
void ethernet_send2(struct network_dev *dev, uint8_t *payload, size_t len, uint16_t ethertype, char dest_mac[6]);
void ethernet_send(struct sockbuf *sb, uint16_t ether_type);

#define ETHERTYPE_IP  0x0800
#define ETHERTYPE_ARP 0x806

int arp_lookup(struct sockbuf *sb, uint32_t ip);
void arp_received(struct sockbuf *sb);

void print_mac(char *mac);
void network_init();
void network_received(struct sockbuf *sbuf);
int sys_network_setup();

uint16_t cksum(uint16_t *buf, size_t nbytes);
#endif
