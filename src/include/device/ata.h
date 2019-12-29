#ifndef C_OS_DEVICE_ATA_H
#define C_OS_DEVICE_ATA_H

#define ATA_IO_BASE 0x1F0
enum {ATA_DATA, ATA_FEATURES, ATA_SEC_CNT, ATA_LBALO, ATA_LBAMID, ATA_LBAHI, ATA_DRVHD, ATA_CMDSTAT};
#define ATA_CTRL 0x3f6
#define nIEN 0x02
#define SRST 0x04
#define HOB  0x80
#define ATA_ER_AMNF		0x01
#define ATA_ER_TK0NF	0x02
#define ATA_ER_ABRT		0x04
#define ATA_ER_MCR		0x08
#define ATA_ER_IDNF		0x10
#define ATA_ER_MC		0x20
#define ATA_ER_UNC		0x40
#define ATA_ER_BBK		0x80
#define ATA_SR_ERR		0x01
#define ATA_SR_IDX		0x02
#define ATA_SR_CORR		0x04
#define ATA_SR_DRQ		0x08
#define ATA_SR_SRV		0x10
#define ATA_SR_DF		0x20
#define ATA_SR_RDY		0x40
#define ATA_SR_BSY		0x80
#define ATA_CMD_READ_PIO	0x20
#define ATA_CMD_WRITE_PIO	0x30
#define ATA_CMD_PACKET		0xA0
#define ATA_CMD_CACHE_FLUSH	0xE7
#define ATA_CMD_IDENTIFY 0xEC

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
	uint16_t start_sector   :6;
	uint16_t start_cylinder :10;
	uint8_t system_id;
	uint8_t end_head;
	uint16_t end_sector   :6;
	uint16_t end_cylinder :10;
	uint32_t rel_sector;
	uint32_t total_sectors;
} __attribute__((packed));

struct mbr {
	uint8_t code[436];
	uint8_t disk[10];
	struct partition_entry partitions[4];
	uint8_t signature[2];
} __attribute__((packed));

struct ata_drive {
	uint16_t io_base;
	struct mbr *mbr;
	struct ata_identify *info;
	char *name;
	uint32_t capacity;
};

// ata_util.c
void ata_dump_identify(struct ata_identify *id);
void print_partition_entry(struct partition_entry *entry);

#endif