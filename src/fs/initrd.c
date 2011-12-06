#include <string.h>
#include <stdio.h>
#include <kernel/memory.h>
#include "vfs.h"
#include "initrd.h"
typedef struct initrd_struct {
	uint8_t *ramdisk;
	uint32_t size;
} initrd_t;
initrd_t initrd0_storage;
initrd_t *initrd0 = &initrd0_storage;

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

int initrd_read_block(/*initrd_t *disk,*/ uint8_t *dst, uint32_t blocknum)
{
	if((blocknum * SECTOR_SIZE) > initrd0->size)
		return -1;
//	printf("%lx %lx\n",(uintptr_t)initrd0->ramdisk,(uintptr_t)(initrd0->ramdisk + (SECTOR_SIZE * blocknum)));
	memcpy(dst, (uint8_t *)(initrd0->ramdisk + (SECTOR_SIZE * blocknum)), SECTOR_SIZE);

	printf("READ BLOCK\n");

	return SECTOR_SIZE;

}

uint8_t bounce[SECTOR_SIZE];

size_t initrd_read(uint16_t dev, void *buf, off_t offset, size_t nbytes)
{
	dev = dev;
	uint32_t index = offset/SECTOR_SIZE;
	uint32_t off = offset % SECTOR_SIZE;
	off = off;
//	int count  = nbytes;
	index = index;
	printf("READ\n"); 
	if(initrd_read_block(bounce, 0) < -1)
		return -1;
	
	kmemcpy((uint8_t *)buf, (uint8_t *)bounce,  nbytes);
	return nbytes;
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
//	char *ptr = (char *)(start);
//	printf("%s\n", ptr);
	struct file * dir = pathsearch(root, "/dev");
	struct file *new = file_new3("initrd", FILE_BLOCK, 0x900);
	insert_file(dir, new);
	printf("INI %X %X\n", initrd_read, initrd_write);

	device_file_register(0x900, initrd_read, initrd_write);

}

