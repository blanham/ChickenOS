/* ChickenOS - device/chardevs.c
 * Implements /dev/(mem|kmem|null|port|zero|full|random|urandom)
 * TODO: Add proper random and implement port access
 * 		 Better access of physical/virtual kernel memory
 * 		 kmem/mem should probably do verification that given
 * 		 	memory is read/writeable
 */
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <chicken/common.h>
#include <chicken/fs/device.h>
#include <chicken/fs/vfs.h>
#include <chicken/mm/vm.h>

size_t chardevs_read(struct inode *inode, uint8_t *buf, size_t count, off_t off)
{
	void *ptr = (void *)(uintptr_t)off;
	dev_t dev = inode->info.st_rdev;
	printf("MAJOR %x dev %x\n", MAJOR(dev), dev);
	ASSERT(MAJOR(dev) == 1, "Bad device passed");
	switch(MINOR(dev))
	{
		case 1: //mem (Physical)
			ptr = P2V(ptr);
		case 2: //kmem (Virtual)
			memcpy(buf, ptr, count);
			return count;
		case 3: //null
			return 0;
		case 4: //port
			return -EINVAL;
		case 7: //full
		case 5: //zero
			memset(buf, 0x00, count);
			return count;
		//FIXME: Add random numbers
		case 8: case 9: //random | urandom
			memset(buf, 4, count);
			return count;
	}
	printf("Invalid read of character device 0x1%.2X\n", MINOR(dev));
	return -EINVAL;
}

size_t chardevs_write(struct inode *inode, uint8_t *buf, size_t count, off_t off)
{
	void *ptr = (void *)(uintptr_t)off;
	dev_t dev = inode->info.st_rdev;
	ASSERT(MAJOR(dev) == 1, "Bad device passed");
	switch(MINOR(dev))
	{
		case 1: //mem (Physical)
			ptr = P2V(ptr);
		case 2: //kmem (Virtual)
			memcpy(ptr, buf, count);
			return count;
		case 4: //port
			return -EINVAL;
		case 3: case 5: case 8: case 9:  //null | zero | random | urandom
			return count;
		case 7: //full
			return -ENOSPC;
	}
	printf("Invalid write of character device 0x1%.2X\n", MINOR(dev));
	return -EINVAL;
}

int chardevs_ioctl(struct inode *inode, int request UNUSED, char *args UNUSED)
{
	dev_t dev = inode->info.st_rdev;
	ASSERT(MAJOR(dev) == 1, "Bad device passed");
	printf("Invalid ioctl of character device 0x1%.2X\n", MINOR(dev));
	return -EINVAL;
}

// FIXME: Pick a better name for this
void chardevs_init()
{
	device_register(S_IFCHR, 0x100, chardevs_read, chardevs_write, chardevs_ioctl);
}