#include <kernel/common.h>
#include <kernel/memory.h>
#include <kernel/console.h>
#include <kernel/hw.h>
#include <kernel/serial.h>
#include <kernel/interrupt.h>
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
struct ata_identify {
	uint16_t gen_config;
	uint16_t num_cylinders;
	uint16_t reserved0;
	uint16_t num_heads;
	uint16_t bytes_per_track;
	uint16_t bytes_per_sector;
	uint16_t sectors_per_track;
	uint16_t vendor[3];
	uint8_t  serial[20];
	uint16_t buffer_type;
	uint16_t buffer_size;
	uint16_t ecc_bytes;
	uint8_t  firmware_rev[8];
	uint16_t model_num[40];
	uint16_t sectors_per_int;
	uint16_t double_io;
	uint16_t capabilities;
	uint16_t reserved1;
	uint16_t pio_mode;
	uint16_t dma_mode;
	uint16_t cyl_lba;
	uint16_t num_cur_cylinders;
	uint16_t num_cur_heads;
	uint16_t num_cur_sectors;
	uint32_t capacity_in_sectors;
	uint16_t reserved2;
	uint32_t user_sectors;
	uint16_t single_dma;
	uint16_t multi_dma;

} __attribute__((packed));
struct partition_entry {
	uint8_t bootable;
	uint8_t start_head;
	unsigned int start_sector :6;
	unsigned int start_cylinder :10;
	uint8_t system_id;
	uint8_t end_head;
	unsigned int end_sector :6;
	unsigned int end_cylinder :10;
	uint32_t rel_sector;
	uint32_t total_sectors;
} __attribute__((packed));
void print_partition_entry(struct partition_entry *entry)
{
	printf("bootable %i start head %i start sector %x total sectors %i type %X\n",
		entry->bootable, entry->start_head, entry->rel_sector, entry->total_sectors*512,entry->system_id);

}

struct mbr {
	uint8_t code[436];
	uint8_t disk[10];
	struct partition_entry partitions[4];
	uint8_t signature[2];

} __attribute__((packed));
struct ata_drive {
	uint16_t io_base;
	struct mbr *mbr;
};
struct ata_drive drives[4];
size_t ata_sector_read(void *buf, unsigned long long lba, uint32_t count UNUSED)
{
	uint16_t *data = buf;
	outb(0x1f6, 0xe0 | ((lba >> 24) & 0xf));
	outb(0x1f1, 0);
	outb(0x1f2, 1);
	outb(0x1f3,lba & 0xff);
	outb(0x1f4,(lba >> 8) & 0xff);
	outb(0x1f5,(lba >> 16) & 0xff);
	outb(0x1f7, 0x20);
	for(int i = 0; i < 256; i++)
		data[i] = inw(0x1f0);

	return 0;
}
size_t ata_read_block(uint16_t dev, void *_buf, uint32_t block_num)
{
	uint8_t part = MINOR(dev) - 1;
	uint8_t drive_num = part/64;
	struct ata_drive *drive = &drives[drive_num];
	struct partition_entry *parti = &drive->mbr->partitions[part];
	//printf("read block %i\n",block_num);
	ata_sector_read(_buf,parti->rel_sector + block_num, 0);
	
	return 512;
}
size_t ata_write(uint16_t dev UNUSED, void *_buf UNUSED, off_t off UNUSED, size_t count UNUSED)
{
	return -1;
}

int ata_ioctl(uint16_t dev UNUSED, int request UNUSED, va_list args UNUSED)
{
	return -1;
}
void ata_intr(struct registers * regs UNUSED)
{
	printf("ata interrupt\n");
}
void ata_detect()
{
	inb(ATA_CTRL);
	inb(ATA_CTRL);
	inb(ATA_CTRL);
	inb(ATA_CTRL);
}
void ata_identify(int drive)
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
	char *name = kcalloc(41, 1);
	for(int i = 27; i < 47; i++)
	{
		uint16_t temp = data[i];// & 0xff00) >> 8;
		data[i] = ((temp >> 8) & 0xff) + ((temp << 8) & 0xff00);	
	}
	kmemcpy(name, p, 40);
//	name[13] = 0;
	printf("Detected HD: %.14s ",name);
	struct ata_identify *st = (void *)data;
	printf("capacity %iMB\n",
	(st->bytes_per_sector*st->sectors_per_track*st->num_cylinders*st->num_heads)/(1024*1024));
	uint32_t logical = *(uint32_t *)&data[117];	
	uint32_t log = *(uint32_t *)&data[60];	
	printf("size %i\n",logical*log);
	drives[drive].io_base = 0x1f0;
	struct mbr *mbr = kcalloc(sizeof(struct mbr),1);
	ata_sector_read(mbr, 0, 1);
	printf("SIG %X %X\n",mbr->signature[0], mbr->signature[1]);
	drives[drive].mbr = mbr;
	print_partition_entry(&mbr->partitions[0]);
	
}
void ata_init()
{
	device_register(FILE_BLOCK, 0x300, ata_read_block, ata_write, ata_ioctl);
	ata_identify(0);
	printf("test %i\n",sizeof(struct partition_entry));

	interrupt_register(0x2E, &ata_intr);
}
