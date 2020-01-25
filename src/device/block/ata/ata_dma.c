#include <stdint.h>
#include <chicken/common.h>
#include <chicken/device/block/ata.h>
//#include <device/pci.h>
//#include <fs/vfs.h>

struct prd {
	uint32_t phys_address;
	uint16_t byte_count;
	// Always zero unless last entry in PRD table
	uint16_t flag;
};