#include <common.h>
#include <stdlib.h>
#include <chicken/boot.h>
#include <chicken/init.h>
#include <chicken/time.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <device/pci.h>
#include <device/usb.h>
#include <device/video.h>
#include <device/tty.h>
#include <device/audio.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <net/net_core.h>
#include <stdio.h>
#include <string.h>

char *BOOT_MSG = "ChickenOS v0.03 booting\n";

void kmain(struct kernel_boot_info *info)
{
	if(info == NULL)
		kernel_halt(); //Something went wrong, hang

//	console_init();

	vm_init(info);

	threading_init();

	interrupt_init();

	pci_init();

	video_init(info);
	tty_init(info);

	//we start out with one color scheme
	//but this will be changed if i ever get a framebuffer
	//console working
//	console_set_color(BLUE,WHITE);
	printf(BOOT_MSG);
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
	vfs_mount_root(ATA0_0_DEV, "ext2");

	extern uint32_t mem_size;
	printf("Found %uMB RAM\n", info->mem_size/1024/1024);

	//init thread is started in user mode
	//so we don't need to switch into user mode
	//thread_create(NULL, init, "ass");
	thread_create2((uintptr_t)init, 0, "Argument");

	//TODO: We should probably use this thread to schedule
	//		bottom halves
	//while(1)
	//	printf("SHIT\n");

	//should probably be a sleep() or something
	kernel_halt();

	//should never return, unless things get really fucked
	PANIC("kmain returned");
}
