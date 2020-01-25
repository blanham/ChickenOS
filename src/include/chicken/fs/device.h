#ifndef C_OS_FS_DEVICE_H
#define C_OS_FS_DEVICE_H
#include <stdint.h>
#include <fcntl.h>
#include <chicken/fs/vfs.h>

/* device.c - device fs */
#define INITRD_DEV 0x400
#define ATA0_0_DEV 0x301
#define ATA0_3_DEV 0x303

#define SECTOR_SIZE 512

#define MAJOR(x) ((x & ~0xFF) >> 8)
#define MINOR(x) (x & 0xFF)

struct device {
    dev_t dev;
    uint16_t type;
    vfs_read_inode_t read;
    vfs_write_inode_t write;
    vfs_ioctl_inode_t ioctl;
};

void device_register(uint16_t type, dev_t dev, vfs_read_inode_t read, vfs_write_inode_t write, vfs_ioctl_inode_t ioctl);
struct device *get_device(uint16_t type, dev_t dev);
inode_t *device_get_inode(uint16_t type, dev_t dev);

#endif