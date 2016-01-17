#include <common.h>
#include <device/usb.h>
#include <device/usb/uhci.h>
#include <device/pci.h>
#include <chicken/time.h>
#include <kernel/hw.h>
#include <mm/vm.h>
#include <stdio.h>
#include <kernel/memory.h>
#include <mm/liballoc.h>

#define PIIX3_USB 0x7020

#define UHCI_IO_USBCMD 		0x00
#define UHCI_IO_USBSTS 		0x02
#define UHCI_IO_USBINTR 	0x04
#define UHCI_IO_FRNUM		0x06
#define UHCI_IO_FLBASEADD	0x08
#define UHCI_IO_SOFMOD		0x0C
#define UHCI_IO_PORTSC1		0x10
#define UHCI_IO_PORTSC2		0x12

#define UHCI_USBCMD_MAXP	0x0080
#define UHCI_USBCMD_CF		0x0040
#define UHCI_USBCMD_SWDBG	0x0020
#define UHCI_USBCMD_FGR		0x0010
#define UHCI_USBCMD_EGSM	0x0008
#define UHCI_USBCMD_GRESET	0x0004
#define UHCI_USBCMD_HCRESET	0x0002
#define UHCI_USBCMD_RS		0x0001

#define UHCI_USBINTR_SPIE	0x0008
#define UHCI_USBINTR_IOCEN	0x0004
#define UHCI_USBINTR_REINEN	0x0002
#define UHCI_USBINTR_CRCOUT	0x0001

#define UHCI_PORTSC_SUSPEND	0x1000
#define UHCI_PORTSC_RESET	0x0200
#define UHCI_PORTSC_LSDA	0x0100
#define UHCI_PORTSC_RESUME	0x0080
#define UHCI_PORTSC_CHANGE	0x0008
#define UHCI_PORTSC_PORTEN	0x0004
#define UHCI_PORTSC_CONNECT	0x0002
#define UHCI_PORTSC_CONSTAT	0x0001

#define UHCI_PCI_LEGACY 	0xC0
#define UHCI_LEG_A20PTS 	0x8000
#define UHCI_LEG_USBPIRQDEN	0x2000
#define UHCI_LEG_USBIRQS	0x1000
#define UHCI_LEG_TBY64W		0x0800
#define UHCI_LEG_TBY64R		0x0400
#define UHCI_LEG_TBY60W		0x0200
#define UHCI_LEG_TBY60R		0x0100
#define UHCI_LEG_SMIEPTE	0x0080
#define UHCI_LEG_PSS		0x0040
#define UHCI_LEG_A20PTEN	0x0020
#define UHCI_LEG_USBSMIEN	0x0010
#define UHCI_LEG_64WEN		0x0008
#define UHCI_LEG_64REN		0x0004
#define UHCI_LEG_60WEN		0x0002
#define UHCI_LEG_50REN		0x0001

void uhci_handler(void *uhci_ptr);

void uhci_outb(struct uhci_controller *uhci, uint16_t port, uint8_t val)
{
	outb(uhci->io_base + port, val);
}

void uhci_outw(struct uhci_controller *uhci, uint16_t port, uint16_t val)
{
	outw(uhci->io_base + port, val);
}

void uhci_outl(struct uhci_controller *uhci, uint16_t port, uint32_t val)
{
	outl(uhci->io_base + port, val);
}

uint16_t uhci_inw(struct uhci_controller *uhci, uint16_t port)
{
	return inw(uhci->io_base + port);
}

uint32_t uhci_inl(struct uhci_controller *uhci, uint32_t port)
{
	return inl(uhci->io_base + port);
}


struct uhci_controller * uhci_init()
{
	struct uhci_controller *new;
	uint32_t ret;

	new = (struct uhci_controller *)kcalloc(sizeof(*new),1);

	//frames need page aligned 4096 byte buffer
	//so we just grab a page.
	new->frames = palloc();
	kmemset(new->frames, 0, PAGE_SIZE);
	new->tx_descs = palloc();
	kmemset(new->tx_descs, 0, PAGE_SIZE);
	new->q_heads = palloc();
	kmemset(new->q_heads, 0, PAGE_SIZE);

	//will eventually want to search by PCI class, but be lazy for now
	//as we are doing all testing in QEMU
	new->pci = pci_get_device(INTEL_VEND, PIIX3_USB);

	if(new->pci != NULL)
	{
		printf("Found Intel PIIX3 UHCI controller Rev %i found at ", new->pci->header->rev);

		new->io_base = pci_get_bar(new->pci, PCI_BAR_IO) & ~1;
		printf("I/O base address %x\n",new->io_base);

		new->mem_base = (uint8_t *)(pci_get_bar(new->pci, PCI_BAR_MEM) & ~3);
		printf("Mem base address %x\t",new->mem_base);

		printf("IRQ %i PIN %i\n",new->pci->header->int_line, new->pci->header->int_pin);


		//get count of ports (8 is max ports)
		for(int i = 0; i < 8; i++)
		{
			ret = uhci_inw(new, UHCI_IO_PORTSC1 + i*2);
			if(((ret & 0x0080) == 0) || (ret == 0xFFFF))
				break;
			new->port_cnt++;
		}

		printf("Found %i ports\n", new->port_cnt);

		//initialize frame list
		for(int i = 0; i < 1024; i++)
		{
			new->frames[i].terminate = 1;
		}

		/* first we need to write to the legacy register in
		 * the PCI config space
		 */
		pci_reg_outw(new->pci, UHCI_PCI_LEGACY/4, 0x8f00);

		//Issue a HCRESET
		uhci_outw(new, UHCI_IO_USBCMD, UHCI_USBCMD_HCRESET);
	//	asm volatile("mfence":::"memory");
		//might need a memory fence and 5us delay here
		//?

		//check if we reset successfully
		ret = uhci_inw(new, UHCI_IO_USBCMD);
		if((ret & UHCI_USBCMD_HCRESET) != 0)
			printf("UHCI reset failed\n");

		//zero out registers
		uhci_outw(new, UHCI_IO_USBINTR, 0);
		uhci_outw(new, UHCI_IO_USBCMD, 0);
		uhci_outw(new, UHCI_IO_PORTSC1, 0);
		uhci_outw(new, UHCI_IO_PORTSC2, 0);

		//init SOFMOD - register determines timing
		uhci_outb(new, UHCI_IO_SOFMOD, 64);

		//initialize frame base pointer and count
		uhci_outl(new, UHCI_IO_FLBASEADD, V2P(new->frames));
		uhci_outw(new, UHCI_IO_FRNUM, 0);

		//fence?

	//	asm volatile("mfence":::"memory");
		//might need this
		pci_reg_outw(new->pci, UHCI_PCI_LEGACY/4, 0x200);
		//fence?

	//	asm volatile("mfence":::"memory");
		uhci_outw(new, UHCI_IO_USBCMD, 1);//UHCI_USBCMD_RS | UHCI_USBCMD_CF | UHCI_USBCMD_MAXP);
		uhci_outw(new, UHCI_IO_USBINTR,0xf);//
			//UHCI_USBINTR_SPIE | UHCI_USBINTR_IOCEN | UHCI_USBINTR_REINEN | UHCI_USBINTR_CRCOUT);


	//	asm volatile("mfence":::"memory");


		//now we need to enable ports
		ret = 0xffff;
		for(int i = 0; i < new->port_cnt; i++)
		{
			uint16_t stat;
			stat  = uhci_inw(new, UHCI_IO_PORTSC1 + i*2);
			if(ret != ((stat & UHCI_PORTSC_CONSTAT) || (stat & UHCI_PORTSC_CHANGE)))
			{
				if((stat & UHCI_PORTSC_CHANGE) != 0)
				{
					uhci_outw(new, UHCI_IO_PORTSC1 + i*2, (stat & ~0xe80a) | UHCI_PORTSC_CHANGE);

				}
				ret = stat;

			}

		//	printf("Status %x\n", ret);

			ret  = uhci_inw(new, UHCI_IO_PORTSC1 + i*2);
			ret |= 1 << 9;
			uhci_outw(new, UHCI_IO_PORTSC1 + i*2, ret);
		//	time_msleep(50);
			//printf("cock\n");
			ret  = uhci_inw(new, UHCI_IO_PORTSC1 + i*2);
			ret &= ~((1 < 9) | (1 << 12));
			uhci_outw(new, UHCI_IO_PORTSC1 + i*2, ret);
			//time_usleep(10);


			ret  = uhci_inw(new, UHCI_IO_PORTSC1 + i*2);
			ret |= 0x4;
			uhci_outw(new, UHCI_IO_PORTSC1 + i*2, ret);


			ret  = uhci_inw(new, UHCI_IO_PORTSC1 + i*2);
		//	printf("Status %x\n", ret);
		}


		pci_register_irq(new->pci, &uhci_handler, new);

		//will want to register callbacks up to usb core here

	}




	return new;
}

void uhci_handler(void *uhci_ptr)
{
	(void)uhci_ptr;
	//struct uhci_controller *uhci = (struct uhci_controller *)uhci_ptr;
	printf("uhci interrupt\n");

}
