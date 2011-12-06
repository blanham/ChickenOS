#include <vfs.h>
#include <kernel/types.h>
#include <stdio.h>


struct device {
	uint16_t dev;
	device_read_fn read;
	device_write_fn write;
	

};

struct device devices[20]; 
void device_file_register(uint16_t dev, device_read_fn read, device_write_fn write)
{
	struct device *sel = &devices[MAJOR(dev)];
	sel->read = read;
	sel->write = write;
	sel->dev = dev;
	printf("REGISTER %x\n", dev);

}

size_t write_major_minor(uint16_t dev, void *buf, int offset, int count)
{

	dev = dev;
	buf = buf;
	offset = offset;
	count = count;


	return -1;

}

size_t device_read(struct file *file, void *buf, off_t offset, size_t nbyte)
{
	size_t ret = 0;
	struct device *dev = &devices[MAJOR(file->device)];
	
	printf("READ %x %X\n", file->device, dev->read);
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

