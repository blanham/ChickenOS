#include <common.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <device/pci.h>
#include <device/usb.h>
#include <device/video.h>
#include <device/audio.h>
#include <device/serial.h>
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
extern void modules_init(struct multiboot_info *mb);
void fork_test();
void init();
extern void multiboot_print(struct multiboot_info *mb);
void ass()
{
	printf("ass\n");
	while(1);
}
//mb is already a virtual address
void kmain(struct multiboot_info* mb, uint32_t magic) 
{
	main_loc = (uintptr_t)&kmain;
   	if ( magic != 0x2BADB002 )
   	{
	//	console_puts("Bad magic number, halting\r");
		return;
   	}
	
	/* begin initializations */
	modules_init(mb);

	vm_init(mb);

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

	serial_init();

	//network_init();

	vfs_init();

	//need to move this back to console.c	
	console_fs_init();

	extern void ata_init();

	ata_init();	
	
	vfs_mount_root(ATA0_0_DEV, "ext2");
	
extern uint32_t mem_size;
	printf("Found %uMB RAM\n", mem_size / 1024);
	thread_usermode();
		
	//we have this special sycall at the moment
	//to setup networking, later will be able to handle
	//it in user space:
	//network_setup();
	//FIXME: probably reuse the above to do a dhcp request
	while(1)
		;
	//	dummy();
	if(!fork() )
	{
		while(1)
			printf("derp\n");
		init();
		PANIC("init() returned!");
	}
	//only works because initial threads name is "main"
	strncpy(thread_current()->name, "idle", 4);
	while(1)
		printf("herp\n");
	//FIXME? needs to be sleep?
	while(1)
		;
		asm volatile("hlt");

	//should never return, unless things get really fucked
	PANIC("kmain returned");
}

//FIXME:Here we should do housekeeping, launch any shells or login processes
//		on the various psuedoterminals, and wait()s on children (which takes
//		care of zombies processes) 
void init()
{
	char *argv[] = {"/frotz","zo",NULL};
	(void)argv;
//		execv("/tests/fork", argv);
		PANIC("execv(init) failed!");

	
	while(1)
		;
}
