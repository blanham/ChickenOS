/*	ChickenOS - init/multiboot.c - multiboot accesory functions
 *	
 */
#include <common.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <device/pci.h>
#include <device/usb.h>
#include <mm/liballoc.h>
#include <multiboot.h>
#include <chicken/boot.h>
#include <chicken/vbe.h>

struct kernel_boot_info multiboot_storage;
extern void *end;
struct kernel_boot_info *multiboot_parse(struct multiboot_info *mb, uint32_t magic)
{
	struct kernel_boot_info *info = &multiboot_storage;
	struct vbe_mode_info *vbe_info;
	uintptr_t temp = 0, placement = (uintptr_t )end;
	kmemsetw((void *)0xC00B8000, 0, 80*25);
	//XXX: Temporary, remove once this is 100%
	extern void serial_init();	
	serial_init();

	if(magic != MULTIBOOT_BOOTLOADER_MAGIC)
		return NULL;

	info->cmdline = (char *)P2V(mb->cmdline);
	info->aux = mb;

	//HACK: Apparently all of the info stuff is loaded last
	
	temp = (uintptr_t)P2V((mb->vbe_mode_info + PAGE_SIZE) & PAGE_MASK);
	serial_printf("But does this one? %x\n", temp);//info->placement);
	//XXX: Temporary modules code	
	extern uint32_t *background_image;
	//kernel_commandline = (char *)(mb->boot_device);
	if(mb->mods_count > 0)
	{
		background_image  = (uint32_t *)P2V(*((void**)P2V(mb->mods_addr)));
		uintptr_t mod_end = (uintptr_t)P2V(*((void**)P2V(mb->mods_addr + 4)));
		serial_printf("%x ", (uint32_t *)P2V(*((void**)P2V(mb->mods_addr))));
		serial_printf(" %x\n", (uint32_t *)P2V(*((void**)P2V(mb->mods_addr + 4))));
		if(mod_end > temp)
			temp = mod_end;
		//	uintptr_t end  = (uintptr_t)*((void **)P2V(mb->mods_addr + 4));
		//	serial_printf("END %X\n", end);
		//	initrd_init(P2V(start),P2V(end));
	}
/*
	if (mb->flags & MULTIBOOT_INFO_MODS)
	{
			multiboot_module_t *mod = (multiboot_module_t *) P2V(mb->mods_addr);
			unsigned i;

			info->modules = (void *)mod;
			info->modules_count = mb->mods_count;

			for (i = 0; i < mb->mods_count; i++)
			{
				mod->mod_start = P2V(mod->mod_start);
				//mod->mod_end += PHYS_BASE;//P2V(mod->mod_end);
				serial_printf("Modend %x\n", &mod->mod_end);
			//	mod->cmdline = P2V(mod->cmdline);
			//	mod++;
			}
	}
*/
	uint32_t high = 0;
	//XXX: This is BAD, please fix
	{
		multiboot_memory_map_t *mmap;
//	serial_printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
//		                   (unsigned) mb->mmap_addr, (unsigned) mb->mmap_length);
	              				
		for (mmap = (multiboot_memory_map_t *) P2V(mb->mmap_addr);
				(unsigned long) mmap < P2V(mb->mmap_addr + mb->mmap_length);
				mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
		{  serial_printf (" size = 0x%x, base_addr = 0x%x,"
				                      " length = 0x%x, type = 0x%x\n",
						                       (unsigned) mmap->size,
								                      (unsigned) mmap->addr,
									                        (unsigned) mmap->len,
										                       (unsigned) mmap->type);	
			if(mmap->len > info->mem_size)
			{
				info->mem_size = mmap->len;
				high = mmap->addr - mmap->size;
			}

		}

	} 
	uintptr_t last_mb_data  =((uintptr_t)&mb->vbe_mode & ~0xFFF) + 4096*8; 
	if(last_mb_data > temp)
		temp = last_mb_data;
	if(temp > placement)
		placement = temp;

	placement = (placement & ~0xFFF) + PAGE_SIZE;

	info->placement = (void *)placement;
	info->mem_size = high - placement;

//	info->mem_size -= (uintptr_t)info->placement - PHYS_BASE;

	vbe_info = (struct vbe_mode_info *)P2V(mb->vbe_mode_info);
	info->framebuffer = (void *)vbe_info->framebuffer_address; 
	serial_printf("Framebuffer %x\n", info->framebuffer);
	
	info->x_res = vbe_info->x_res;
	info->y_res = vbe_info->y_res;
	info->bpp = vbe_info->bpp;
	info->mode = mb->vbe_mode;
	if(info->mode == 0)
	{
		info->y_res = 25;
		info->x_res = 80;
	}

	return info;
}
