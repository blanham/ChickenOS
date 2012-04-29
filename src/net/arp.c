


void send_arp()
{
/*
struct rtl8139 *rtl = global;
	struct ether_header *eh;
	struct ether_arp *arp;
	uint8_t *buf =kmalloc(sizeof(*eh) + sizeof(*arp));
	
	void* tx_buffer = (void *)(rtl->tx_buffers + 8192*rtl->tx_cur);
	eh = (void *)buf;
	arp = (void *)(buf + sizeof(*eh));
	for(int i = 0; i < 6; i++)
	{
		eh->ether_shost[i] = rtl_inb(rtl, i);

	}
	for(int i = 0; i < 6; i++)
	{
		eh->ether_dhost[i] = 0xff;//rtl_inb(rtl, i);

	}
	for(int i = 0; i < 6; i++)
	{
		arp->arp_sha[i] = rtl_inb(rtl, i);

	}
	for(int i = 0; i < 6; i++)
	{
		arp->arp_tha[i] = 0;//dest_mac[i];

	}
	for(int i = 0; i < 4; i++)
	{

		arp->arp_spa[i] = our_ip[i];
		arp->arp_tpa[i] = their_ip[i];

	}
  //  kmemcpy(arp->arp_spa, our_ip, 4);
	// Ethertype field 
    eh->ether_type = htons(0x806);//0x680;//htons(ETH_P_IP);
	arp->type = htons(1);
	arp->proto = htons(0x806);
	arp->hlen = 6;//htons(6);
	arp->plen = 4;//htons(4);
	arp->op = htons(1);




*/








}
