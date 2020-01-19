#include <common.h>
#include <sys/stat.h>
#include <string.h>
// FIXME: Move this header?
#include <fs/initrd.h>
#include <fs/vfs.h>

//XXX: This is very old code originally use for the initrd, needs cleanup and
//     renaming, but leaving for now.

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

int initrd_write_block(void *_disk UNUSED, void *src, uint32_t blocknum)
{
	if((blocknum * SECTOR_SIZE) > initrd0->size)
		return -1;
	memcpy((uint8_t *)(initrd0->ramdisk + (SECTOR_SIZE * blocknum)), src, SECTOR_SIZE);

	return SECTOR_SIZE;

}

void initrd_init(uintptr_t start, uintptr_t end)
{
	initrd0->ramdisk = (uint8_t *)start;
	initrd0->size = end - start;
	printf("Initializing initrd @ %x - %i bytes \n",start, initrd0->size);

	//device_register(S_IFBLK, 0x400, initrd_read_block, initrd_write_block, NULL);

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