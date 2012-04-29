#ifndef C_OS_NET_NETCORE_H
#define C_OS_NET_NETCORE_H
#include <kernel/types.h>
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
struct socket {
	struct network_dev *dev;


};
struct addrinfo {
    int     ai_flags;
    int     ai_family;
    int     ai_socktype;
    int     ai_protocol;
    size_t  ai_addrlen;
    struct  sockaddr *ai_addr;
    char    *ai_canonname;     /* canonical name */
    struct  addrinfo *ai_next; /* this struct can form a linked list */
};

int getaddrinfo(const char *node,
                const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
 
void freeaddrinfo(struct addrinfo *res);
int socket(int domain, int type, int protocol); 

void ethernet_send(struct network_dev *dev, uint8_t *payload, size_t len, uint16_t ethertype, char dest_mac[6]);
void udp_send(struct network_dev *dev, uint16_t src, uint16_t dst, 
	uint8_t *payload, size_t len);
size_t ip_send(struct network_dev *dev, uint8_t proto,uint8_t *payload, size_t len);


void print_mac(char *mac);
void network_init();



	
#endif
