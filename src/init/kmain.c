#include <common.h>
#include <stdlib.h>
#include <chicken/boot.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <device/pci.h>
#include <device/usb.h>
#include <device/video.h>
#include <device/audio.h>
#include <kernel/timer.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <fs/initrd.h>
#include <net/net_core.h>
#include <stdio.h>
#include <string.h>
#include <thread/syscall.h>

char *BOOT_MSG = "ChickenOS v0.03 booting\n";
void init(void *aux UNUSED) __attribute__((section(".user"))); 

void kmain(struct kernel_boot_info *info)
{
	if(info == NULL)
		while(1) //Something went wrong, hang
			;	

	vm_init(info);

	thread_init();

	interrupt_init();

//	console_init();

	pci_init();

	video_init(info);

	//we start out with one color scheme
	//but this will be changed if i ever get a framebuffer
	//console working
//	console_set_color(BLUE,WHITE);
	console_puts(BOOT_MSG);

	audio_init();
	
	extern void pci_list();
//	pci_list();	

	kbd_init();	

	time_init();

	syscall_init();
	
	interrupt_enable();
	
	usb_init();

	extern void ata_init();
	ata_init();	

	vfs_init();

	//network_init();
	
	//need to move this back to console.c	
	console_fs_init();

	//TODO: move this to a mount_root() function
	//		can take drive, but should autodetect
	//		filesytem type from the partition table
	vfs_mount_root(ATA0_0_DEV, "ext2");

	extern uint32_t mem_size;
	printf("Found %uMB RAM\n", mem_size / 1024);

	//init thread is started in user mode
	//so we don't need to switch into user mode	
	thread_create(NULL,(void *)init, NULL);

	//TODO: We should probably use this thread to schedule
	//		bottom halves 
	
	//should probably be a sleep() or something
	kernel_halt();

	while(1)
		;
	//should never return, unless things get really fucked
	PANIC("kmain returned");
}
//Wut?
void exit(int code UNUSED)
{
	PANIC("exit() called!");
}
//FIXME:Here we should do housekeeping, launch any shells or login processes
//		on the various psuedoterminals, and wait()s on children (which takes
//		care of zombies processes) 
void init(void *aux UNUSED) 
{
	char *argv[] = {"/sh", NULL};
	char *path = "/dash";
	(void)argv;
	if(!fork())
	{
	//we have this special sycall at the moment
	//to setup networking, later will be able to handle
	//it in user space:
	//network_setup();
	//FIXME: probably reuse the above to do a dhcp request
	//	dummy();
		SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
		SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
		SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);

		execv(path, argv);
		PANIC("execv(init) failed!");
	}

	while(1)
		;
}
