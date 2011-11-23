#include "vfs.h"
#include <stdint.h>

struct fat_bootblock {
	uint8_t jmp[3];
	uint8_t desc[8];
	uint16_t bytes_per_block;
	uint8_t  blocks_per_alloc;
	uint16_t reserved_blocks;
	uint8_t	 num_fats;
	uint16_t root_directories;
	uint16_t disk_blocks;//0 if overflow
	uint8_t  media_desc;
	uint16_t fat_blocks;
	uint16_t blocks_per_track;
	uint16_t heads;
	uint32_t hidden_blocks;
	uint32_t disk_blocks2;
	uint16_t drive_number;
	uint8_t  boot_record;
	uint32_t volume_serial;
	uint32_t volume_label;
	uint8_t  file_system[8];
	uint8_t  boot[0x1c2];
	uint16_t signature; 
} __attribute__ (packed);

typedef struct fat_bootblock fat_bootblock_t;
