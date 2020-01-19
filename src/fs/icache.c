#include <common.h>
#include <stdbool.h>
//#include <utlist.h>
#include <string.h>
#include <mm/liballoc.h>
#include <fs/icache.h>
#include <fs/vfs.h>
#include <sys/stat.h>
#include <stdio.h>

// XXX: This needs to be cleaned up and implemented

// FIXME: This should probably dynamically grow
#define BCACHE_SIZE 2048
struct icache {
//	struct lock *free_lock, cache_lock;
	int count;

	//hash table
	struct inode *cache;
	int keylen;

	// XXX: Might be worth having the pointer for this be a union, or re-use the hash pointer
	struct inode *free_list;
} icache_storage, *inode_cache = &icache_storage;

int icache_init()
{
	struct inode *new;
	memset(inode_cache, 0, sizeof *inode_cache);

	//FIXME: Use better allocation later
	for(int i = 0; i < BCACHE_SIZE; i++)
	{
		new = kcalloc(sizeof *new, 1);
		//In kernel if this happens we will panic
		//so don't bother cleaning up
		if(new == NULL)
			return -1;

		//DL_APPEND(inode_cache->free_list, new);
	}

	//init hash table
	inode_cache->cache = NULL;
	//inode_cache->keylen = offsetof(struct inode, ino)
	//					+ sizeof (uint32_t)
	//					- offsetof(struct inode, dev);


	return 0;
}

struct inode *icache_alloc(dev_t dev, ino_t inode)
{
	struct inode *ret;
	// Meowman may i play the game now?
	//lol get free list lock
	ret = inode_cache->free_list;
	if(ret == NULL)
		PANIC("derp");
	ret->device = dev;
	ret->number = inode;

	//DL_DELETE(inode_cache->free_list, ret);
	//release free list lock
	//HASH_ADD(hh, inode_cache->cache, dev, inode_cache->keylen, ret);

	return ret;
}

struct inode *icache_get(dev_t device, ino_t inode)
{
	(void)device;
	(void)inode;
/*	if in_hash
		increase ref
		return it

*/
	return NULL;
}

void icache_put(struct inode *inode)
{
	if(inode == NULL)
		return;

	(void)inode;
}
