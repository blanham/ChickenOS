#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/memory.h>


struct icmp {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t rest;	
} __attribute__((packed));
