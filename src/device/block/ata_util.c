#include <common.h>
#include <stdint.h>
#include <device/ata.h>

void ata_dump_identify(struct ata_identify *id)
{
    serial_printf("gen_config: %2X cylinders: %i reserved: %i heads: %i bytes/track: %X bytes/sector %X sectors/track %X\n",
	    id->gen_config, id->num_cylinders, id->reserved0, id->num_heads, id->bytes_per_track, id->bytes_per_sector, id->sectors_per_track);
    serial_printf("Vendor %X%X%X serial %.20s buffer type: %x size: %x ecc: %x firmware %.8s\n",
    	id->vendor[0], id->vendor[1], id->vendor[2], &id->serial, id->buffer_type, id->buffer_size, id->ecc_bytes, &id->firmware_rev);
    serial_printf("Model: %.80s\n", (uint8_t *)&id->model_num);
    serial_printf("sectors/int %X double_io: %X capabilities %X reserved: %X pio mode: %X dma_mode %X\n",
    	id->sectors_per_int, id->double_io, id->capabilities, id->reserved1, id->pio_mode, id->dma_mode);
    serial_printf("LBA Cylinders %X curcylinders %X curheads %X cursectors %X #sectors %X total_sectors %x\n",
	    id->cyl_lba, id->num_cur_cylinders, id->num_cur_heads, id->num_cur_sectors, id->capacity_in_sectors);
    serial_printf("Reservec %X usersectors: %X single dma: %X multi dma: %X\n",
	    id->reserved2, id->user_sectors, id->single_dma, id->multi_dma);
}

void print_partition_entry(struct partition_entry *entry)
{
	serial_printf("bootable %i start head %i start sector %x total sectors %i type %X\n",
		entry->bootable, entry->start_head, entry->rel_sector, entry->total_sectors*512,entry->system_id);
}