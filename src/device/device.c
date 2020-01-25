/*	ChickenOS - fs/device.c - generic block/char device layer
 *	Needs a bit of work
 */
#include <common.h>
#include <memory.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fs/device.h>
#include <fs/vfs.h>
#include <mm/liballoc.h>

#define MAX_DEVICES 20

struct device block_devices[MAX_DEVICES];
struct device char_devices[MAX_DEVICES];

void device_register(uint16_t device_type, dev_t dev, vfs_read_inode_t read, vfs_write_inode_t write, vfs_ioctl_inode_t ioctl)
{
	struct device *device = NULL;

	if (device_type == S_IFCHR) {
		device = &char_devices[MAJOR(dev)];
	} else if(device_type == S_IFBLK) {
		device = &block_devices[MAJOR(dev)];
	}

	device->read  = read;
	device->write = write;
	device->ioctl = ioctl;
	device->dev = dev; //MAJOR(dev); // XXX: ?

	// TODO: log devices registered:
	//printf("Registered device %x:%x\n", MAJOR(dev),MINOR(dev));
}


/*
inode_t *device_get_inode(uint16_t type, dev_t dev);
{
	struct device *inode = inode_;

	if (device_type == S_IFCHR) {
		ret = &char_devices[MAJOR(dev)];
	} else if(device_type == S_IFBLK) {
		ret = &block_devices[MAJOR(dev)];
	}

	return ret
}*/

struct device *get_device(uint16_t device_type, dev_t dev)
{
	struct device *ret = NULL;

	if (device_type == S_IFCHR) {
		ret = &char_devices[MAJOR(dev)];
	} else if(device_type == S_IFBLK) {
		ret = &block_devices[MAJOR(dev)];
	}

	return ret;
}