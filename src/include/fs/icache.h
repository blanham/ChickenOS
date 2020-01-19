#ifndef C_OS_FS_ICACHE_H
#define C_OS_FS_ICACHE_H
//include whatever defines blk_t and dev_t
//#include <uthash.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>


struct inode_key {
	dev_t device;
	ino_t inode;
};

struct inode *icache_get(dev_t dev, ino_t ino);
struct inode *icache_alloc(dev_t dev, ino_t inode);
/*block_t *block_get(dev_t dev, uint32_t block);
int block_put(block_t *block);
int block_cache_init();
int block_cache_flush();*/
int icache_init();
void icache_put(struct inode *);
#endif