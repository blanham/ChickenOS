#ifndef C_OS_DEVICE_ATA_H
#define C_OS_DEVICE_ATA_H
#include <stdbool.h>

enum ata_cmds {
	ATA_REG_DATA,
	ATA_REG_FEATURES,	// Feature register, write only
	ATA_REG_ERROR = 1,	// Error register, read only
	ATA_REG_SEC_CNT,
	ATA_REG_LBALO,
	ATA_REG_LBAMID,
	ATA_REG_LBAHI,
	ATA_REG_DRVHD,		// Drive/head select, r/w
	ATA_REG_CMD,		// Command register, write only
	ATA_REG_STATUS = 7,	// Status register, read only (resets interrupts)
	ATA_CTRL_ALTSTAT = 0, // Alternate status regsiter, read only (no effect on interrupts)
	ATA_CTRL_DEVCTRL = 0, // Device control register, write only
	ATA_CTRL_DEVADDR = 1, // Device address regsiter, read only
};

#define ATA_IO_BASE 0x1F0
#define ATA_CTRL	0x3F6

#define ATA_DEV_CTRL_nIEN 0x02
#define ATA_DEV_CTRL_SRST 0x04
#define ATA_DEV_CTRL_HOB  0x80

#define ATA_ERR_AMNF	0x01
#define ATA_ERR_TK0NF	0x02
#define ATA_ERR_ABRT	0x04
#define ATA_ERR_MCR		0x08
#define ATA_ERR_IDNF	0x10
#define ATA_ERR_MC		0x20
#define ATA_ERR_UNC		0x40
#define ATA_ERR_BBK		0x80

#define ATA_DRVHD_ADDR	0x0F // Bottom 4 bits are for setting head #/LBA[24:27]
#define ATA_DRVHD_DRV	0x10 // Set for slave drive
#define ATA_DRVHD_LBA	0x40 // Set for slave drive
#define ATA_DRVHD_SET	0xA0 // Always set

#define ATA_STAT_ERR	0x01
#define ATA_STAT_IDX	0x02
#define ATA_STAT_CORR	0x04
#define ATA_STAT_DRQ	0x08
#define ATA_STAT_SRV	0x10
#define ATA_STAT_DF		0x20
#define ATA_STAT_RDY	0x40
#define ATA_STAT_BSY	0x80

#define ATA_CMD_READ_PIO				0x20
#define ATA_CMD_WRITE_PIO				0x30
#define ATA_CMD_PACKET					0xA0
#define ATA_CMD_IDENTIFY_PACKET_DEVICE	0xA1
#define ATA_CMD_CACHE_FLUSH				0xE7
#define ATA_CMD_IDENTIFY				0xEC

typedef struct ata_identify {
	uint16_t gen_config; 
	uint16_t num_cylinders;
	uint16_t reserved0; // specific configuration

	// I think this info is old
	uint16_t num_heads;
	uint16_t bytes_per_track;
	uint16_t bytes_per_sector;
	uint16_t sectors_per_track;

	uint16_t vendor[3];
	uint8_t  serial[20];

	// More old fields
	uint16_t buffer_type;
	uint16_t buffer_size;
	uint16_t ecc_bytes;

	uint8_t  firmware_rev[8];
	uint16_t model_num[20];
	uint16_t sectors_per_int; // I think this is maximum number of sectors you can transfer in one interrupt
				              // but MS has this as two separate uint8_t fields

	uint16_t double_io; // MS calls this "Trusted Computing"??

	uint16_t capabilities[2]; // Does this need to be checked? Has LBA and DMA supported bits

	// More obsolete fields, might be needed for old hardware support
	uint16_t pio_mode;
	uint16_t dma_mode;

	// Dunno if this is right
	uint16_t cyl_lba;

	// What do these current fields mean?
	uint16_t num_cur_cylinders;
	uint16_t num_cur_heads;
	uint16_t num_cur_sectors; // current sectors per track
	uint32_t capacity_in_sectors;

	uint16_t reserved2; // rw mult size?

	uint32_t user_sectors; // User addressable sectors LBA28


	uint16_t single_dma;
	uint16_t multi_dma;
	uint8_t pio_modes;
	uint8_t reserved_3;
	uint16_t more_reserved[35];

	uint64_t sectors_lba48;

	uint16_t more_crap[152];
} __attribute__((packed)) ata_id_t;

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

typedef struct ata_drive {
	ata_id_t identity;
	struct mbr mbr;
	uint16_t io_base;
	uint16_t ctrl_base;
	bool slave;
	char name[81];
	uint32_t capacity;
} ata_dev_t;

// ata_util.c
void ata_busy_wait(struct ata_drive *dev);
uint16_t ata_port_read(struct ata_drive *dev, uint16_t port);
void ata_port_write(struct ata_drive *dev, uint16_t port, uint16_t value);
inline void ata_ctrl_write(struct ata_drive *dev, uint8_t port, uint8_t value);
inline uint8_t ata_ctrl_read(struct ata_drive *dev, uint8_t port);

void ata_dump_identify(struct ata_identify *id);
void print_partition_entry(struct partition_entry *entry);
#endif