#ifndef C_OS_FS_FAT_FAT_H
#define C_OS_FS_FAT_FAT_H

struct fat16_bootblock {
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
	uint32_t sectors_per_partition;
	
	uint8_t logical_drive;
	uint8_t unused;
	uint8_t ext_sig;
	uint32_t volume_serial;
	uint8_t volume_label[11];
	uint8_t  file_system[8];
	uint8_t  boot[448];
	uint16_t signature; 
} __attribute__ ((packed));
struct fat_directory_entry {
	uint8_t filename[11];
	uint8_t attrib;
	uint8_t reserved;
	uint8_t tenths;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t accesse_date;
	uint16_t cluster_high;
	uint16_t modified_time;
	uint16_t modified_date;
	uint16_t cluster_low;
	uint32_t file_size;
} __attribute__ ((packed));
typedef struct fat_directory_entry directory_t;
typedef struct fat16_bootblock fat16_bootblock_t;
typedef struct fat16_bootblock fat12_bootblock_t;

struct fat32_bootblock {
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
	uint32_t sectors_per_partition;
	uint32_t sectors_per_fat;
	uint16_t flags;
	uint16_t fatver;
	uint32_t root_cluster;
	uint16_t fs_info_sector;
	uint16_t backup_boot_sector;
	uint8_t reserved[12];
	uint8_t logical_drive;
	uint8_t unused;
	uint8_t ext_sig;
	uint32_t volume_serial;
	uint8_t volume_label[11];
	uint8_t  file_system[8];
	uint8_t  boot[420];
	uint16_t signature; 
} __attribute__ ((packed));

typedef struct fat32_bootblock fat32_bootblock_t;

struct partition_entry {
	uint8_t bootflag;
	uint8_t chs_begin[3];
	uint8_t typecode;
	uint8_t chs_end[3];
	uint32_t lba_begin;
	uint32_t num_sectors;
} __attribute__ ((packed));
typedef struct partition_entry partition_t;

struct mbr_struct {
	uint8_t bootcode[446];
	partition_t parts[4];
	uint16_t signature;
} __attribute__ ((packed));

typedef struct mbr_struct mbr_t;
typedef struct fat_filesystem {
	//fat_bootblock_t *bootblock;
	int type;


} fat_filesystem_t;


#endif
