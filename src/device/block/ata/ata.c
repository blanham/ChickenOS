#include <common.h>
#include <memory.h>
#include <chicken/time.h>
#include <device/ata.h>
#include <device/pci.h>
#include <fs/vfs.h>
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <errno.h>
#include <sys/stat.h>

// TODO: Add spinlock to protect r/w sector calls

static struct ata_drive drives[4] = {
	{.io_base = 0x1F0, .ctrl_base = 0x3F6},
	{.io_base = 0x1F0, .ctrl_base = 0x3F6, .slave = true},
	{.io_base = 0x170, .ctrl_base = 0x376},
	{.io_base = 0x170, .ctrl_base = 0x376, .slave = true},
};

enum intr_status ata_intr_status;
void ata_lock()
{
	ata_intr_status = interrupt_disable();
}

void ata_unlock()
{
	interrupt_set(ata_intr_status);
}

void ata_pio_cmd2(ata_dev_t *dev, uint8_t command, uint64_t lba, size_t count)
{
	uint8_t select_query = ATA_DRVHD_SET | (dev->slave ? ATA_DRVHD_DRV : 0);

	if (command == ATA_CMD_READ_PIO || command == ATA_CMD_WRITE_PIO) {
		select_query |= ATA_DRVHD_LBA | ((lba >> 24) & 0xf);
	}

	ata_port_write(dev, ATA_REG_DRVHD, select_query);
	ata_port_write(dev, ATA_REG_SEC_CNT, (uint8_t)count);
	ata_port_write(dev, ATA_REG_LBALO,  (uint8_t)(lba & 0xff));
	ata_port_write(dev, ATA_REG_LBAMID, (uint8_t)((lba >> 8) & 0xff));
	ata_port_write(dev, ATA_REG_LBAHI,  (uint8_t)((lba >> 16) & 0xff));
	ata_port_write(dev, ATA_REG_CMD, command);

	ata_busy_wait(dev);

	if (ata_port_read(dev, ATA_REG_STATUS) == 0) {
		int error = ata_port_read(dev, ATA_REG_ERROR);
		printf("Error %X returned while trying to execute command %X\n", error, command);
		PANIC("ATA PIO command failed!");
		return;
	}

	// XXX: This shouldn't busy wait, will fix later
	while((ata_port_read(dev, ATA_REG_STATUS) & (ATA_STAT_BSY | ATA_STAT_DRQ)) != ATA_STAT_DRQ);
}


size_t ata_sector_read(struct ata_drive *dev, void *buf, unsigned long long lba, uint32_t count)
{
	// XXX: Whatever is calling this is setting the count to 0 heh
	if (count == 0) {
		count = 1;
	}
	ata_pio_cmd2(dev, ATA_CMD_READ_PIO, lba, count);

	uint16_t *data = buf;
	for (unsigned i = 0; i < count; i++) {
		for(unsigned i = 0; i < 256; i++)
			*data++ = ata_port_read(dev, ATA_REG_DATA);

		//ata_busy_wait(dev);
		while((ata_port_read(dev, ATA_REG_STATUS) & (ATA_STAT_BSY)))
			;
	}

	return 512 * count;
}

size_t ata_sector_write(struct ata_drive *dev, void *buf, unsigned long long lba, uint32_t count)
{

	ata_pio_cmd2(dev, ATA_CMD_WRITE_PIO, lba, count);

	uint16_t *data = buf;
	for(int i = 0; i < 256; i++)
	{
		//FIXME: should be delayed
		// XXX: Is that right?
		//but this doesn't work, interrupt problem?
		//time_msleep(1);
		ata_port_write(dev, ATA_REG_DATA, data[i]);
	}

	// Some drives require a cache flush after a write to function properly
	ata_port_write(dev, ATA_REG_CMD, ATA_CMD_CACHE_FLUSH);
	// XXX: Do we need to wait for that to complete?

	return 512 * count;
}

/*
size_t ata_read_block2(struct inode *inode, uint8_t *buf, size_t count, off_t block_num)
{
	struct ata_drive *drive = inode->storage;
	return 0;
}
*/

size_t ata_read_blocks(uint16_t dev, void *_buf, uint32_t block_num, size_t blocks)
{
	uint8_t part = MINOR(dev) - 1;
	uint8_t drive_num = part/64;
	struct ata_drive *drive = &drives[drive_num];
	struct partition_entry *parti = &drive->mbr.partitions[part];

	return ata_sector_read(drive, _buf, parti->rel_sector + block_num, blocks);
}

// FIXME: this is a wrapper from when I changed the prototype to use an inode, clean up when I can
size_t ata_read_thing(struct inode *inode, uint8_t *buf, size_t count, off_t offset)
{
	uint32_t lba = offset / SECTOR_SIZE;
	size_t blocks = count / SECTOR_SIZE; 
	return ata_read_blocks(inode->info.st_dev, buf, lba, blocks);
}

size_t ata_write_block(uint16_t dev, void *_buf, uint32_t block_num)
{
	uint8_t part = MINOR(dev) - 1;
	uint8_t drive_num = part/64;
	struct ata_drive *drive = &drives[drive_num];
	struct partition_entry *parti = &drive->mbr.partitions[part];

	ata_sector_write(drive, _buf, parti->rel_sector + block_num, 0);

	return 512;
}

int ata_ioctl(struct inode *inode UNUSED, int request UNUSED, char *args UNUSED)
{
	// https://www.kernel.org/doc/Documentation/ioctl/hdio.txt
	return -ENXIO;
}

void ata_interrupt(void * _unused UNUSED)
{
	ata_port_read(&drives[0], ATA_REG_STATUS);
}

void ata_interrupt_secondary(void * __unused UNUSED)
{
	ata_port_read(&drives[3], ATA_REG_STATUS);
}

static inline void bswap16(uint16_t *ptr, size_t count)
{
	while (count--) {
		uint16_t temp = *ptr;
		*ptr++ = ((temp >> 8) & 0xff) + ((temp << 8) & 0xff00);
	}
}

int ata_pio_cmd(ata_dev_t *dev, uint8_t command)
{
	ata_port_write(dev, ATA_REG_DRVHD, ATA_DRVHD_SET | ATA_DRVHD_LBA | ( dev->slave ? ATA_DRVHD_DRV : 0) | 0);// ((lba >> 24) & 0xf));
	ata_port_write(dev, ATA_REG_SEC_CNT, 0);
	ata_port_write(dev, ATA_REG_LBALO, 0);
	ata_port_write(dev, ATA_REG_LBAMID, 0);
	ata_port_write(dev, ATA_REG_LBAHI, 0);
	ata_port_write(dev, ATA_REG_CMD, command);

	ata_busy_wait(dev);

	if (ata_port_read(dev, ATA_REG_STATUS) == 0) {

		int error = ata_port_read(dev, ATA_REG_ERROR);
		printf("Error %X returned while trying to execute command %X\n", error, command);
		//PANIC("ATA PIO command failed!");
		return 1;
	}
	// XXX: Or here?
	//while((ata_port_read(dev, ATA_CMDSTAT) & ATA_STAT_BSY) != 0);

	return 0;
}

void ata_device_init(ata_dev_t *dev)
{
	ata_id_t *id = &dev->identity;
	uint16_t *id_ptr = (uint16_t *)id;

	if (ata_pio_cmd(dev, ATA_CMD_IDENTIFY))
		return;

	// XXX: Here?
	while((ata_port_read(dev, ATA_REG_STATUS) & ATA_STAT_BSY) != 0);

	// XXX: this should use a repsw inw on x86
	for(int i = 0; i < 256; i++)
		id_ptr[i] = ata_port_read(dev, ATA_REG_DATA);

	// Byteswap vendor, serial, firmware rev, and model #
	bswap16(&id_ptr[7], 13);
	bswap16(&id_ptr[23], 24);

	// Dump id to serial log
	ata_dump_identify(id);

	kmemcpy(&dev->name, id->model_num, 40);

	// FIXME: No LBA48 support yet
	//if (id->sectors_lba48) {
	//	dev->capacity = id->sectors_lba48 * id->bytes_per_sector;
	//} else 
	
	if (id->user_sectors) {
		dev->capacity = id->user_sectors * id->bytes_per_sector;
	} else {
		dev->capacity = id->bytes_per_sector * id->sectors_per_track 
						* id->num_cylinders * id->num_heads;
	}

	// NOTE: Use PIO mode so we don't rely on interrupts for now
	ata_sector_read(dev, &dev->mbr, 0, 1);
	for (int i = 0; i < 4; i++) {
		print_partition_entry(&dev->mbr.partitions[i]);
	}
}

void ata_reset_bus(struct ata_drive *dev)
{
	ata_port_write(dev, dev->ctrl_base, 0x04);
	ata_busy_wait(dev);
	ata_port_write(dev, dev->ctrl_base, 0x00);
	ata_busy_wait(dev);
}

void ata_detect_and_initialize(struct ata_drive *dev)
{
	ata_reset_bus(dev);

	// Select the drive
	ata_port_write(dev, ATA_REG_DRVHD, ATA_DRVHD_SET | (dev->slave ? ATA_DRVHD_DRV : 0));
	ata_busy_wait(dev);

	// Wait for bus to be free
	while((ata_port_read(dev, ATA_REG_STATUS) & ATA_STAT_BSY) != 0);

	// ATA Status register returns 0xFF if the bus is empty, so we return
	if (ata_port_read(dev, ATA_REG_STATUS) == 0xFF) {
		return;
	}

	uint16_t signature = ata_port_read(dev, ATA_REG_LBAMID) | ata_port_read(dev, ATA_REG_LBAHI) << 8;
	switch (signature) {
		case 0x0000:
		case 0xC33C:
			ata_device_init(dev);
			printf("Detected ATA drive: %.14s Capacity: %iMB\n", dev->name, dev->capacity/(1024*1024));
			break;
		case 0xEB14:
		case 0x9669:
			printf("Detected ATAPI device: %x\n", signature);
			return;
		default:
			printf("Invalid or unsupported ATA device: %x\n", signature);
		case 0xFFFF: 
			return;
	}
}

void ata_init()
{
	// XXX: Can't I do this at compile time?
	ASSERT(sizeof(ata_id_t) == SECTOR_SIZE, "ATA identify structure wrong size!");

	// TODO: This should look for Class 0x0101 
	struct pci_device *intel_ata = pci_get_device(INTEL_VEND, 0x7010);

	if (intel_ata != NULL) {
		uint32_t io_base = pci_get_barn(intel_ata, PCI_BAR_IO, 4) & ~1;
		printf("Detected PCI Bus Mastering DMA IDE interface @ 0x%.8X\n", io_base);
		device_register(S_IFBLK, 0x300, ata_read_thing, NULL, ata_ioctl);
	} else {
		printf("No PCI DMA, falling back to PIO mode\n");
		device_register(S_IFBLK, 0x300, ata_read_thing, NULL, ata_ioctl);
	}

	interrupt_register(IRQ14, (void *)&ata_interrupt);
	interrupt_register(IRQ15, (void *)&ata_interrupt_secondary);

	for (int i = 0; i < 4; i++)
		ata_detect_and_initialize(&drives[i]);
}