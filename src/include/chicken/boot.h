#ifndef C_OS_CHICKEN_BOOT_H
#define C_OS_CHICKEN_BOOT_H
#include <stdint.h>
struct hardware_mem_region {
	uint32_t size;
	uint64_t addr;
	uint64_t len;
	#define MEMORY_AVAILABLE	1
	#define MEMORY_RESERVED		2
	uint32_t type;
} __attribute__((packed));

struct boot_module 
{
 	uint32_t mod_start;
	//First byte after end of module
 	uint32_t mod_end;
  	char* cmdline;
 	uint32_t pad;
} __attribute__((packed));

struct kernel_boot_info {
	char *cmdline;
	uintptr_t mem_size;
	void *placement; //beginning of heap
	//entry
	//commandline
	//memory size
	//uint64_t mem_size;
	//regions
	struct hardware_mem_region *regions;
	uint32_t modules_count;
	struct boot_module *modules;
	//memsize
	//memmap?
	//modules?

	void *framebuffer;
	uint32_t x_res, y_res, bpp;
	uint32_t mode;
	void *aux;
};

#endif
