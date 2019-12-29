#include <kernel/common.h>
#include <sys/stat.h>
#include <chicken/time.h>
#include <kernel/memory.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <mm/liballoc.h>
#include <fs/vfs.h>
#include <device/ata.h>

struct ata_drive drives[4];

//XXX: Why is this switch statement like this?
void ata_write(struct ata_drive *dev, uint16_t port, uint16_t value)
{
	switch(port)
	{
		case 0:
			outw(dev->io_base + port, value);
			break;
		case 1 ... 7:
			outb(dev->io_base + port, value);
			break;
		default:
			outb(port, value);
			break;
	}
}

uint16_t ata_read(struct ata_drive *dev, uint16_t port)
{
	switch(port)
	{
		case 0:
			return inw(dev->io_base + port);
		case 1 ... 7:
			return inb(dev->io_base + port);
		default:
			return inb(port);
}
	return 0;
}

size_t ata_sector_read(struct ata_drive *dev, void *buf, unsigned long long lba, uint32_t count UNUSED)
{
	uint16_t *data = buf;

	ata_write(dev, ATA_DRVHD, 0xe0 | ((lba >> 24) & 0xf));
	ata_write(dev, ATA_FEATURES, 0);

	ata_write(dev, ATA_SEC_CNT, 1);

	ata_write(dev, ATA_LBALO,  (uint8_t)(lba & 0xff));
	ata_write(dev, ATA_LBAMID, (uint8_t)((lba >> 8) & 0xff));
	ata_write(dev, ATA_LBAHI,  (uint8_t)((lba >> 16) & 0xff));

	ata_write(dev, ATA_CMDSTAT, ATA_CMD_READ_PIO);
	int t = 1;
	while(t)
	{
		int a = ata_read(dev, ATA_CMDSTAT);

		if(((a & 0x80) == 0) && ((a & 8) == 8))
			break;
	}
	for(int i = 0; i < 256; i++)
		data[i] = ata_read(dev, ATA_DATA);

	return 512;
}

size_t ata_sector_write(struct ata_drive *dev, void *buf, unsigned long long lba, uint32_t count UNUSED)
{
	uint16_t *data = buf;

	ata_write(dev, ATA_DRVHD, 0xe0 | ((lba >> 24) & 0xf));
	ata_write(dev, ATA_FEATURES, 0);

	ata_write(dev, ATA_SEC_CNT, 1);

	ata_write(dev, ATA_LBALO,  (uint8_t)(lba & 0xff));
	ata_write(dev, ATA_LBAMID, (uint8_t)((lba >> 8) & 0xff));
	ata_write(dev, ATA_LBAHI,  (uint8_t)((lba >> 16) & 0xff));

	ata_write(dev, ATA_CMDSTAT, ATA_CMD_WRITE_PIO);

	for(int i = 0; i < 256; i++)
	{
		//FIXME: should be delayed
		//but thus doesn't work, interrupt problem?
		//time_msleep(1);
		ata_write(dev, ATA_DATA, data[i]);
	}

	ata_write(dev, ATA_CMDSTAT, ATA_CMD_CACHE_FLUSH);

	return 512;
}

size_t ata_read_block(uint16_t dev, void *_buf, uint32_t block_num)
{
	uint8_t part = MINOR(dev) - 1;
	uint8_t drive_num = part/64;
	struct ata_drive *drive = &drives[drive_num];
	struct partition_entry *parti = &drive->mbr->partitions[part];

	ata_sector_read(drive, _buf,parti->rel_sector + block_num, 0);

	return 512;
}

size_t ata_write_block(uint16_t dev, void *_buf, uint32_t block_num)
{
	uint8_t part = MINOR(dev) - 1;
	uint8_t drive_num = part/64;
	struct ata_drive *drive = &drives[drive_num];
	struct partition_entry *parti = &drive->mbr->partitions[part];

	ata_sector_write(drive, _buf, parti->rel_sector + block_num, 0);

	return 512;
}


int ata_ioctl(uint16_t dev UNUSED, int request UNUSED, va_list args UNUSED)
{
	// https://www.kernel.org/doc/Documentation/ioctl/hdio.txt
	return -1;
}

void ata_intr(struct registers * regs UNUSED)
{
	//printf("ata interrupt\n");
}

void ata_identify(int drive)
{
	uint16_t *data;
	uint16_t io_base;
	struct ata_drive *dev;
	struct ata_identify *st;

	dev = &drives[drive];
	st = dev->info = kcalloc(256, 2);
	dev->name = kcalloc(41, 1);
	data = (uint16_t *)dev->info;

	switch(drive)
	{
		case 0:
			io_base = ATA_IO_BASE;
		break;
	}

	dev->io_base = io_base;
	ata_write(dev, ATA_DRVHD, 0xA0);
	ata_write(dev, ATA_SEC_CNT, 0);
	ata_write(dev, ATA_LBALO, 0);
	ata_write(dev, ATA_LBAMID, 0);
	ata_write(dev, ATA_LBAHI, 0);
	ata_write(dev, ATA_CMDSTAT, ATA_CMD_IDENTIFY);

	ata_read(dev, ATA_CTRL);
	ata_read(dev, ATA_CTRL);
	ata_read(dev, ATA_CTRL);
	ata_read(dev, ATA_CTRL);

	if(ata_read(dev, ATA_CMDSTAT) == 0)
	{
		printf("Error returned while trying to IDENTIFY\n");
		return;
	}

	while((ata_read(dev, ATA_CMDSTAT) & ATA_SR_BSY) != 0);

	for(int i = 0; i < 256; i++)
		data[i] = ata_read(dev, ATA_DATA);

	//have to byteswap the name
	for(int i = 10; i < 20; i++)
	{
		uint16_t temp = data[i];// & 0xff00) >> 8;
		data[i] = ((temp >> 8) & 0xff) + ((temp << 8) & 0xff00);
	}
	for(int i = 23; i < 47; i++)
	{
		uint16_t temp = data[i];// & 0xff00) >> 8;
		data[i] = ((temp >> 8) & 0xff) + ((temp << 8) & 0xff00);
	}

	ata_dump_identify((void *)data);

	kmemcpy(dev->name, dev->info->model_num, 40);
	dev->capacity = st->bytes_per_sector*st->sectors_per_track*st->num_cylinders*st->num_heads;

	printf("Detected HD: %.14s Capacity: %iMB\n", dev->name, dev->capacity/ (1024*1024));

	dev->mbr = kcalloc(sizeof(struct mbr),1);
	ata_sector_read(dev, dev->mbr, 0, 1);

}
void ata_init()
{
	device_register(S_IFBLK, 0x300, ata_read_block, ata_write_block, ata_ioctl);
	interrupt_register(IRQ14, &ata_intr);
	ata_identify(0);
//	printf("test %i\n",sizeof(struct partition_entry));
}
