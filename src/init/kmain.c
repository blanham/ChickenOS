#include <common.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <device/pci.h>
#include <device/usb.h>
#include <device/video.h>
#include <device/audio.h>
#include <device/tty.h>
#include <kernel/timer.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <fs/initrd.h>
#include <net/net_core.h>
#include <stdio.h>
#include <string.h>
#include <thread/syscall.h>

char *BOOT_MSG = "ChickenOS v0.02 booting\n";

uintptr_t main_loc;

void init(void *aux UNUSED) __attribute__((section(".user"))); 
extern void modules_init(struct multiboot_info *mb);
void fork_test();
extern void multiboot_print(struct multiboot_info *mb);

//TODO: need to change signature since ARM has no multiboot
//      (unless I write a stub that takes info from u-boot 
//		and shoves it into a multiboot struct) 
//mb is already a virtual address
void kmain(struct multiboot_info* mb, uint32_t magic) 
{
	main_loc = (uintptr_t)&kmain;
   	if ( magic != 0x2BADB002 )
   	{
	//	console_puts("Bad magic number, halting\r");
		return;
   	}
	
	#ifdef ARCH_I386
	/* begin initializations */
	modules_init(mb);
/*
	Instead of the preceding and following functions taking the mb
	have a function called boot_info_parse that calls accessor functions:
		vm_init_info
		modules_init_info
	tht are boot info agnostic
	so then you have
	multiboot_init(mb):
		struct mem_extents extents
			^put shit from mb strudt into that
		vm_init_info(extents)
	OR
		just a commandline + parser	
*/
	vm_init(mb);
	
	tty_init();

	thread_init();

	interrupt_init();

//	console_init();

	pci_init();

	video_init();

	//we start out with one color scheme
	//but this will be changed if i ever get a framebuffer
	//console working
//	console_set_color(BLUE,WHITE);
	console_puts(BOOT_MSG);
//	multiboot_print(mb);
	audio_init();
	
	extern void pci_list();

//	pci_list();	

	kbd_init();	

	time_init();

	syscall_init();
	
	interrupt_enable();
	
	usb_init();


	//network_init();

	vfs_init();

	//need to move this back to console.c	
	console_fs_init();

	extern void ata_init();

	ata_init();	
	
	//TODO: move this to a mount_root() function
	//		can take drive, but should autodetect
	//		filesytem type from the partition table
	vfs_mount_root(ATA0_0_DEV, "ext2");

	extern uint32_t mem_size;
	printf("Found %uMB RAM\n", mem_size / 1024);

	//init thread is started in user mode
	//so we don't need to switch into user mode	
	thread_create(NULL,(void *)init, NULL);

	//restarts interrupts, allowing new thread to be
	//scheduled
	interrupt_enable();	

	//should probably be a sleep() or something
	kernel_halt();
	

	while(1)
		;
	#elif ARCH_ARM
	(void)mb;
	while(1)
		;
	#endif
	//should never return, unless things get really fucked
	PANIC("kmain returned");
}

//FIXME:Here we should do housekeeping, launch any shells or login processes
//		on the various psuedoterminals, and wait()s on children (which takes
//		care of zombies processes) 
void init(void *aux UNUSED) 
{
	char *argv[] = {"/busybox","vi",NULL};
	char *path = "/busybox";
	(void)argv;
	if(!fork())
	{
	//we have this special sycall at the moment
	//to setup networking, later will be able to handle
	//it in user space:
	//network_setup();
	//FIXME: probably reuse the above to do a dhcp request
	//	dummy();
		SYSCALL_3N(SYS_OPEN, "/dev/tty", 0, NULL);
		SYSCALL_3N(SYS_OPEN, "/dev/tty", 0, NULL);
		SYSCALL_3N(SYS_OPEN, "/dev/tty", 0, NULL);

		execv(path, argv);
		PANIC("execv(init) failed!");
	}

	while(1)
		;
}
