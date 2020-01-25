#include <common.h>
#include <stdint.h>
#include <device/ata.h>
#include <kernel/hw.h>
//XXX: Why is this switch statement like this?
inline void ata_port_write(struct ata_drive *dev, uint16_t port, uint16_t value)
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

inline uint16_t ata_port_read(struct ata_drive *dev, uint16_t port)
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

inline void ata_ctrl_write(struct ata_drive *dev, uint8_t port, uint8_t value)
{
	outb(dev->ctrl_base + port, value);
}

inline uint8_t ata_ctrl_read(struct ata_drive *dev, uint8_t port)
{
	return inb(dev->ctrl_base + port);
}

void ata_busy_wait(struct ata_drive *dev)
{
	ata_port_read(dev, ATA_CTRL);
	ata_port_read(dev, ATA_CTRL);
	ata_port_read(dev, ATA_CTRL);
	ata_port_read(dev, ATA_CTRL);
}

void ata_dump_identify(ata_id_t *id)
{
    serial_printf("gen_config: %2X cylinders: %i reserved: %i heads: %i bytes/track: %X bytes/sector %X sectors/track %X\n",
	    id->gen_config, id->num_cylinders, id->reserved0, id->num_heads, id->bytes_per_track, id->bytes_per_sector, id->sectors_per_track);
    serial_printf("Vendor %X%X%X serial %.20s buffer type: %x size: %x ecc: %x firmware %.8s\n",
    	id->vendor[0], id->vendor[1], id->vendor[2], &id->serial, id->buffer_type, id->buffer_size, id->ecc_bytes, &id->firmware_rev);
    serial_printf("Model: %.80s\n", (uint8_t *)&id->model_num);
    serial_printf("sectors/int %X double_io: %X capabilities %X:%X pio mode: %X dma_mode %X\n",
    	id->sectors_per_int, id->double_io, id->capabilities[0], id->capabilities[1], id->pio_mode, id->dma_mode);
    serial_printf("LBA Cylinders %X curcylinders %X curheads %X cursectors %X #sectors %X LBA48 sectors %X total_sectors %x\n",
	    id->cyl_lba, id->num_cur_cylinders, id->num_cur_heads, id->num_cur_sectors, id->sectors_lba48, id->capacity_in_sectors);
    serial_printf("Reservec %X usersectors: %X single dma: %X multi dma: %X\n",
	    id->reserved2, id->user_sectors, id->single_dma, id->multi_dma);
}

void print_partition_entry(struct partition_entry *entry)
{
	serial_printf("bootable %i start head %i start sector %x total sectors %i type %X\n",
		entry->bootable, entry->start_head, entry->rel_sector, entry->total_sectors,entry->system_id);
}