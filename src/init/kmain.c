#include <stdio.h>
#include <chicken/boot.h>
#include <chicken/common.h>
#include <chicken/device/pci.h>
#include <chicken/device/tty.h>
#include <chicken/device/video.h>
#include <chicken/fs/device.h>
#include <chicken/fs/vfs.h>
#include <chicken/init.h>
#include <chicken/thread.h>
#include <chicken/time.h>


#include <chicken/lib/rbtree.h>

#define VERSION "v0.1.0"
char *BOOT_MSG = "\033[32;1mChickenOS "VERSION" booting\033[0m\n";

void kmain(struct kernel_boot_info *info)
{
	if(info == NULL)
		kernel_halt(); //Something went wrong, hang

#ifndef ARCH_AARCH64
	vm_init(info);
	video_init(info);
//	console_init();
	tty_init(info);

	printf(BOOT_MSG);

	threading_init();

	interrupt_init();

	pci_init();

//	audio_init();

	extern void pci_list();
//	pci_list();

	kbd_init();

	time_init();

	interrupt_enable();

//	usb_init();

	extern void ata_init();
	ata_init();

	vfs_init();

	//network_init();

	//TODO: move this to a mount_root() function
	//		can take drive, but should autodetect
	//		filesytem type from the partition table
	// FIXME: this is just for my convenience for testing, remove when we have better ways of specifying the device
	dev_t root_device = ATA0_0_DEV;
	if (info->mem_size/1024/1024 >= 254) {
		root_device = ATA0_3_DEV;
	}

	vfs_mount_root(root_device, "ext2");

	printf("Found %uMB RAM\n", info->mem_size/1024/1024);
	//printf("Lower: %X Upper: %X\n", info->low_mem, info->hi_mem);

	thread_start_init(init, NULL);

	// TODO: Add bottom halves/software interupts, other kernel threads, etc

#endif
	kernel_halt();

	//should never return, unless things get really fucked
	PANIC("kmain returned");
}
