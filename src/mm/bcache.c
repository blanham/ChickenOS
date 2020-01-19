#include <common.h>
#include <stdio.h>
#include <string.h>
#include <chicken/bcache.h>
#include <chicken/boot.h>


typedef struct {
	union {
		uintptr_t addr;
		void *ptr;
	};
    size_t block_size;
    size_t block_sizes[4];
	// block size?
	// shift?
	uint64_t number;
	// use a bitmap for blocks larger than pages/sectors
	uint64_t sparse_bitmap;
	union {
        uint32_t block_nums[8];
		//dev_t device;
	};
	atomic_int ref_count;
} __attribute__((packed)) block_t;




block_t *block_get()
{
    block_t *ret = NULL;

    return ret;
}

void bcache_init(struct kernel_boot_info *info)
{
    (void)info;

}

