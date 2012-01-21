#include <kernel/common.h>
#include <kernel/memory.h>
#include <kernel/console.h>
#include <kernel/hw.h>
#include <kernel/serial.h>
#include <kernel/vm.h>
#include <mm/liballoc.h>
#include <fs/vfs.h>
#define ATA_IO_BASE 0x1F0
enum {ATA_DATA, ATA_FEATURES, ATA_SEC_CNT, ATA_LBALO, ATA_LBAMID, ATA_LBAHI, ATA_DRVHD, ATA_CMDSTAT};
#define ATA_CTRL 0x3f6
#define nIEN 0x02
#define SRST 0x04
#define HOB  0x80
#define ERR 0x01
#define DRQ 0x08
#define SRV 0x10
#define DF  0x20
#define RDY 0x40
#define BSY 0x80
size_t ata_read(uint16_t dev, void *_buf UNUSED, off_t off UNUSED, size_t count UNUSED)
{
	uint8_t part = MINOR(dev);
	printf("trying to read from device %i partiton %i\n",part/64, part);
	return -1;
}
size_t ata_write(uint16_t dev UNUSED, void *_buf UNUSED, off_t off UNUSED, size_t count UNUSED)
{
	return -1;
}

int ata_ioctl(uint16_t dev UNUSED, int request UNUSED, va_list args UNUSED)
{
	return -1;
}
/*void ata_intr(struct registers * regs UNUSED)
{
}*/
void ata_detect()
{
	inb(ATA_CTRL);
	inb(ATA_CTRL);
	inb(ATA_CTRL);
	inb(ATA_CTRL);
}
void ata_identify()
{
	uint16_t *data = kcalloc(256, 2);
	outb(0x1f6, 0xA0);
	outb(0x1f2, 0);
	outb(0x1f3, 0);
	outb(0x1f4, 0);
	outb(0x1f5, 0);
	
	outb(0x1f7, 0xec); 
	inb(ATA_CTRL);
	inb(ATA_CTRL);
	inb(ATA_CTRL);
	inb(ATA_CTRL);

	int test = inb(0x1f7);
	if(test == 0)
	{
		printf("fail\n");
		return;
	}
	while((test = inb(0x1f7)) & 0x80);
	for(int i = 0; i < 256; i++)
		data[i] = inw(0x1f0);
	
	char *p = (void *)&data[27];
//	uint8_t temp = 0;
	char *name = kcalloc(41, 1);
	name[40] = '\0';
	for(int i = 27; i < 47; i++)
	{
		uint16_t temp = data[i];// & 0xff00) >> 8;
		data[i] = ((temp >> 8) & 0xff) + ((temp << 8) & 0xff00);	
	}
	kmemcpy(name, p, 40);
	name[13] = 0;
	printf("Detected HD: %s\n",name);	
}
void ata_init()
{
	device_register(FILE_BLOCK, 0x300, ata_read, ata_write, ata_ioctl);
	ata_identify();
	//interrupt_register(0x21, &ata_intr);
}
