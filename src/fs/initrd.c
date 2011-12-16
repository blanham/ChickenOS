#include <string.h>
#include <stdio.h>
#include <kernel/common.h>
#include <kernel/memory.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/initrd.h>
typedef struct initrd_struct {
	uint8_t *ramdisk;
	uint32_t size;
} initrd_t;
initrd_t initrd0_storage;
initrd_t *initrd0 = &initrd0_storage;


int initrd_read_block(void *_disk UNUSED, void *dst, uint32_t blocknum)
{
	if((blocknum * SECTOR_SIZE) > initrd0->size)
		return -1;
	memcpy(dst, (uint8_t *)(initrd0->ramdisk + (SECTOR_SIZE * blocknum)), SECTOR_SIZE);

	return SECTOR_SIZE;

}

uint8_t bounce[SECTOR_SIZE];

size_t initrd_read(uint16_t dev UNUSED, void *buf, off_t offset, size_t nbytes)
{
	uint32_t index = offset/SECTOR_SIZE;
	uint32_t off = offset % SECTOR_SIZE;
	off = off;
	printf("READ\n");
	size_t count = 0;
	while(nbytes > 0)
	{ 
		if(nbytes % SECTOR_SIZE)
		{	
			if(initrd_read_block(NULL, bounce, index) < -1)
				return -1;
		
			kmemcpy((uint8_t *)buf, (uint8_t *)bounce,  nbytes);
		}else{



			buf += SECTOR_SIZE;
		}
		nbytes -= SECTOR_SIZE;
		index++;
		count += SECTOR_SIZE;	
	}
	return count;
}
size_t initrd_write(uint16_t dev, void *buf, off_t offset, size_t nbytes)
{
	dev = dev;
	uint32_t index = offset;
	index = index; 
//	if(initrd_read_block(bounce, index ) < -1)
//		return -1;
	
	kmemcpy((uint8_t *)buf, (uint8_t *)bounce,  nbytes);
	return nbytes;
}

void initrd_init(uintptr_t start, uintptr_t end)
{
	initrd0->ramdisk = (uint8_t *)start;
	initrd0->size = end - start;
	printf("Initializing initrd @ %x - %i bytes \n",start, initrd0->size);

	device_register(FILE_BLOCK,0x400, initrd_read_block, initrd_write);

}

/*void initrd_tester(char *filename)
{

	FILE *fp = fopen(filename, "rb");
	uint8_t *ramdisk;
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("SIZE %x\n",(uint32_t)size);
	ramdisk = malloc(size);
	fread(ramdisk, size, 1, fp);
	if(ramdisk == NULL)
		printf("fuck\n");
	initrd_init((uintptr_t)ramdisk, (uintptr_t)ramdisk + (uintptr_t)size);
	fclose(fp);

}
*/

