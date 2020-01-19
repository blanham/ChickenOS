#include <kernel/common.h>
#include <sys/stat.h>
#include <chicken/time.h>
#include <device/pci.h>
#include <kernel/memory.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <mm/liballoc.h>
#include <fs/vfs.h>
#include <device/ata.h>

struct prd {
	uint32_t phys_address;
	uint16_t byte_count;
	// Always zero unless last entry in PRD table
	uint16_t flag;
};