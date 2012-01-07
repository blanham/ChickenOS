#include <common.h>
#include <kernel/console.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/timer.h>
#include <kernel/types.h>
#include <kernel/vm.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/initrd.h>
#include <stdio.h>
#include <string.h>
#include <thread/syscall.h>

char *BOOT_MSG = "ChickenOS v0.01 booting\n";

extern int sh_main(void *aux);
extern void print_mb(unsigned long addr, unsigned long magic);

void modules_init(struct multiboot_info *mb)
{
	if(mb->mods_count > 0 )
	{
		uintptr_t start = (uintptr_t)*((void**)P2V(mb->mods_addr));
		uintptr_t end  = (uintptr_t)*((void **)P2V(mb->mods_addr + 4));
		initrd_init(P2V(start),P2V(end));
	}
}

void kmain(uint32_t mbd, uint32_t magic)
{
	struct multiboot_info *mb = 0;
   	if ( magic != 0x2BADB002 )
   	{
		console_puts("Bad magic number, halting\r");
		return;
   	}
 	
	mb = (struct multiboot_info *)P2V(mbd);
		
	/* begin initializations */
	interrupt_init();
	vm_init(mb);
	console_init();
	paging_init();

	//we start out with one color scheme
	//but this will be changed if i ever get a framebuffer
	//console working
	console_set_color(BLUE,WHITE);
	console_puts(BOOT_MSG);
	
	time_init();
	syscall_init();
	thread_init();
//	print_mb((uint32_t)mbd, magic);

	if(1)
	{
		modules_init(mb);	
		vfs_init();
		//need to move this back to console.c	
		console_fs_init();
	
		vfs_mount_root(INITRD_DEV, "ext2");
	}

	thread_usermode();
	
	extern pid_t fork();	
	char *argv[] = {"-l", "foo", "bar", NULL};	
	if(!fork() )
	{
		execv("/test", argv);
		PANIC("execv(init) failed!");	
	}
	//only works because initial threads name is "main"
	strcpy(thread_current()->name, "idle");
	while(1)
		;

	//should never return, unless things get really fucked
	PANIC("kmain returned");
}

