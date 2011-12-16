#include <kernel/common.h>
#include <kernel/memory.h>
#include <kernel/fs/vfs.h>
#include <kernel/types.h>
#include <mm/liballoc.h>
#include <stdio.h>

#define MAX_DEVICES 20
#define NULL 0
struct char_device {
	uint16_t dev;
	char_read_fn read;
	char_write_fn write;
};
struct block_device {
	uint16_t dev;
	block_read_fn read;
	block_write_fn write;
};

struct block_device block_devices[MAX_DEVICES]; 
struct char_device char_devices[MAX_DEVICES]; 

void device_register(uint16_t device_type, uint16_t dev, void *read, void *write)
{
	if(device_type == FILE_CHAR)
	{
		struct char_device *c = &char_devices[MAJOR(dev)];
		c->read = (char_read_fn)read;
		c->write = (char_write_fn)write;
		c->dev = MAJOR(dev);

	}
	else if(device_type == FILE_BLOCK)
	{
		struct block_device *b = &block_devices[MAJOR(dev)];
		b->read = (block_read_fn )read;
		b->write = (block_write_fn )write;
		b->dev = MAJOR(dev);

	}
	
	printf("REGISTER %x\n", dev);

}

size_t block_device_read(uint16_t dev, void *buf, uint32_t block)
{
	struct block_device *device = &block_devices[MAJOR(dev)];
	if(device == NULL)
	{
		printf("invalid dev passed to block_device_read\n");
		return 0;
	}
	if(device->read == NULL)
	{
		printf("device has no read function\n");
		return 0;
	}
	
	return device->read(dev, buf, block);

}

size_t block_device_readn(uint16_t dev, void *buf, uint32_t block, off_t offset, size_t nbyte)
{
	return read_block_generic(buf, nbyte, block*offset, 
		SECTOR_SIZE, (void *)(uint32_t)dev, 
		(block_access_fn)block_device_read);

}
/*
size_t device_read(struct file *file, void *buf, off_t offset, size_t nbyte)
{
	size_t ret = 0;
	struct device *dev = &devices[MAJOR(file->device)];
	
	printf("READ %x %X %s\n", file->device, dev->read, file->name);
	ret = dev->read(file->device, buf, offset, nbyte);

	return ret;
}
size_t device_write(struct file *file, void *buf, off_t offset, size_t nbyte)
{
	size_t ret = 0;
	struct device *dev = &devices[MAJOR(file->device)];
	ret = dev->write(file->device, buf, offset, nbyte);

	return ret;
}
*/


int read_block_generic(void * _buf, int size, int offset, int block_size, void *aux, block_access_fn f UNUSED)
{
	uint8_t *buf = _buf;
	uint8_t *bounce;
	off_t count = 0;
	if(block_size <= 0)
		return -1;
	
	if((bounce = kmalloc(block_size)) == NULL)
	{
		printf("memory allocation error in readblock\n");
		return -1;

	}
//	printf("offset %i size%i block_size%i\n", offset, size,block_size);
//	printf("block = %i\n",offset/block_size);
	while(size > 0)
	{
		int block = offset / block_size;
		int block_ofs = offset % block_size;
		int cur_size = block_size - block_ofs;
	//	printf("reading block %i\n",block);	
		if(block_ofs == 0 && cur_size == block_size)
		{

			if(block_device_read((uint16_t)(uint32_t)aux, buf + count, block) == 0)
			{
				count = -1;
				goto end;
			}

		}
		else
		{
			if(block_device_read((uint16_t)(uint32_t)aux, bounce, block) == 0)
			{
				count = -1;
				goto end;
			}

			kmemcpy(buf + count, bounce + block_ofs, cur_size);
		} 

		count  += cur_size;
		offset += cur_size;
		size   -= cur_size;
	}

end:
	//FIXME
	//free(bounce);
	return count;
}


int read_block(uint16_t dev, void * _buf, int block, int block_size)
{
	uint8_t *buf = _buf;
	off_t count = 0;
	if(block_size <= 0)
		return -1;
	int dblock = block*block_size / SECTOR_SIZE;
	printf("dev %x %x %x %x %i\n",dev, _buf, block,block_size, dblock);	
	while(block_size > 0)
	{

		if(block_device_read(dev, (void *)(buf + count), dblock) != SECTOR_SIZE)
		{
				count = -1;
				goto end;
		}

		count  += SECTOR_SIZE;
		block_size   -= SECTOR_SIZE;
		dblock++;
	}

end:
	return count;
}
