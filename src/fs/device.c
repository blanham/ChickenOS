/*	ChickenOS - fs/deivce.c - generic block/char device layer
 *	Needs a bit of work
 */
//Ideally we'll have 2 classes of functions
//[block|char]_[read|write]_at -> takes offset/size
//block_[read|write] -> takes block only, used for lowlevel
//access
//then we can have a common struct device again

#include <kernel/common.h>
#include <kernel/memory.h>
#include <fs/vfs.h>
#include <mm/liballoc.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_DEVICES 20
#define NULL 0

struct char_device {
	dev_t dev;
	char_read_fn read;
	char_write_fn write;
	char_ioctl_fn ioctl;
};
struct block_device {
	dev_t dev;
	block_read_fn read;
	block_write_fn write;
	void *pad;
};

struct block_device block_devices[MAX_DEVICES]; 
struct char_device char_devices[MAX_DEVICES]; 

void device_register(uint16_t device_type, dev_t dev, void *read, void *write, void *ioctl)
{
	if(device_type == FILE_CHAR)
	{
		struct char_device *c = &char_devices[MAJOR(dev)];
		c->read = (char_read_fn)read;
		c->write = (char_write_fn)write;
		c->ioctl = (char_ioctl_fn)ioctl;
		c->dev = MAJOR(dev);

	}
	else if(device_type == FILE_BLOCK)
	{
		struct block_device *b = &block_devices[MAJOR(dev)];
		b->read = (block_read_fn )read;
		b->write = (block_write_fn )write;
		b->dev = MAJOR(dev);

	}
	
	printf("Registered device %x:%x\n", MAJOR(dev),MINOR(dev));

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
size_t block_device_write(uint16_t dev, void *buf, uint32_t block)
{
	struct block_device *device = &block_devices[MAJOR(dev)];
	if(device == NULL)
	{
		printf("invalid dev passed to block_device_write\n");
		return 0;
	}
	if(device->write == NULL)
	{
		printf("device has no write function\n");
		return 0;
	}
	
	return device->write(dev, buf, block);

}

size_t block_device_readn(uint16_t dev, void *buf, uint32_t block, off_t offset, size_t nbyte)
{
	return read_block_at(dev, buf, block,SECTOR_SIZE,
		offset, nbyte);
}
size_t block_device_writen(uint16_t dev, void *buf, uint32_t block, off_t offset, size_t nbyte)
{
	return write_block_at(dev, buf, block,SECTOR_SIZE,
		offset, nbyte);
}


size_t char_device_read(uint16_t dev, void *buf, off_t offset, size_t nbyte)
{
	size_t ret = 0;
	struct char_device *device = &char_devices[MAJOR(dev)];
	
//	printf("READ %x %X\n", dev, device->read);
	ret = device->read(dev, buf, offset, nbyte);

	return ret;
}
size_t char_device_write(uint16_t dev, void *buf, off_t offset, size_t nbyte)
{
	size_t ret = 0;
	struct char_device *device = &char_devices[MAJOR(dev)];
	ret = device->write(dev, buf, offset, nbyte);

	return ret;
}

int char_device_ioctl(uint16_t dev, int request, ...)
{
	size_t ret = 0;
	struct char_device *device = &char_devices[MAJOR(dev)];
	ret = device->ioctl(dev, request, NULL);

	return ret;
}


int read_block(uint16_t dev, void * _buf, int block, int block_size)
{
	uint8_t *buf = _buf;
	uint32_t dblock = 0;
	off_t count = 0;
	
	if(block_size <= 0)
		return -1;
	dblock = (block * block_size) / SECTOR_SIZE;

	while(block_size > 0)
	{
		if(block_device_read(dev, (void *)(buf + count), dblock) != SECTOR_SIZE)
		{
			return -1;
		}

		count  		+= SECTOR_SIZE;
		block_size  -= SECTOR_SIZE;
		dblock++;
	}

	return count;
}


int read_block_at(uint16_t dev, void * _buf, int block, 
	int block_size, off_t offset, size_t nbytes)
{
	uint8_t *buf = _buf;
	uint8_t *bounce = kmalloc(block_size);

	int remaining = nbytes;
	size_t count = 0;
	int block_ofs, cur_block_size, 
		till_end;
	int cur_size = offset;
	while(remaining > 0)
	{
		block = (block*block_size + cur_size) / block_size; 
		block_ofs = offset % block_size;
		cur_block_size = block_size - block_ofs;
	//	to_end = 1000000000;
	//	till_end = (to_end < cur_block_size) ? to_end: cur_block_size;
		till_end = cur_block_size;
		cur_size = remaining < till_end ? remaining : till_end;
		if(cur_size <= 0)
			break;
		
		if(block_ofs == 0 && cur_size == block_size)
		{
			if(read_block(dev, buf + count, block,block_size)  == 0)
			{
				count = -1;
				goto done;
			}
		}
		else
		{
			if(read_block(dev, buf + count, block,block_size)  == 0)
			{
				count = -1;
				goto done;
			}

			kmemcpy(buf + count, bounce + block_ofs,cur_size);
		} 

		count  += cur_size;
		offset += cur_size;
		remaining   -= cur_size;
	}

done:
	if(bounce != NULL)
		kfree(bounce);
	
	return count;
}


int write_block(uint16_t dev, void * _buf, int block, int block_size)
{
	uint8_t *buf = _buf;
	uint32_t dblock = 0;
	off_t count = 0;
	
	if(block_size <= 0)
		return -1;
	
	dblock = (block * block_size) / SECTOR_SIZE;

	while(block_size > 0)
	{
		if(block_device_write(dev, buf + count, dblock) != SECTOR_SIZE)
		{
			return -1;
		}

		count  		+= SECTOR_SIZE;
		block_size  -= SECTOR_SIZE;
		dblock++;
	}

	return count;
}


int write_block_at(uint16_t dev, void * _buf, int block, 
	int block_size, off_t offset, size_t nbytes)
{
	uint8_t *buf = _buf;
	uint8_t *bounce = kmalloc(block_size);

	int remaining = nbytes;
	size_t count = 0;
	int block_ofs, cur_block_size, 
		till_end;
	int cur_size = offset;
	while(remaining > 0)
	{
		block = (block*block_size + cur_size) / block_size; 
		block_ofs = offset % block_size;
		cur_block_size = block_size - block_ofs;
	//	to_end = 1000000000;
	//	till_end = (to_end < cur_block_size) ? to_end: cur_block_size;
		till_end = cur_block_size;
		cur_size = remaining < till_end ? remaining : till_end;
		if(cur_size <= 0)
			break;
		
		if(block_ofs == 0 && cur_size == block_size)
		{
			if(write_block(dev, buf + count, block,block_size)  == 0)
			{
				count = -1;
				goto done;
			}
		}
		else
		{
			//have to read the block in first
			if(block_ofs > 0 || cur_size < cur_block_size)
			{

				read_block(dev, bounce, block, block_size);
			}
			kmemcpy(bounce + block_ofs,buf + count,cur_size);
			if(write_block(dev, buf + count, block,block_size)  == 0)
			{
				count = -1;
				goto done;
			}

		} 

		count  += cur_size;
		offset += cur_size;
		remaining   -= cur_size;
	}

done:
	if(bounce != NULL)
		kfree(bounce);
	
	return count;
}

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
	//kfree(bounce);
	return count;
}




