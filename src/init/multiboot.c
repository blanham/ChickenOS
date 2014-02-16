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

void modules_init(struct multiboot_info *mb);

struct kernel_boot_info *multiboot_parse(struct multiboot_info *mb, uint32_t magic)
{
	struct kernel_boot_info *info = &multiboot_storage;
	struct vbe_mode_info *vbe_info;

	//Do this here for error shit
	extern void serial_init();	
	serial_init();

	if(magic != MULTIBOOT_BOOTLOADER_MAGIC)
		return NULL;

	info->cmdline = (char *)P2V(mb->cmdline);
	info->aux = mb;

	modules_init(mb);

	if (mb->flags & MULTIBOOT_INFO_MODS)
	{
		multiboot_module_t *mod = (multiboot_module_t *) P2V(mb->mods_addr);
		unsigned i;

		info->modules = (void *)mod;
		info->modules_count = mb->mods_count;

		for (i = 0; i < mb->mods_count; i++)
		{
			 mod->mod_start = P2V(mod->mod_start);
			 mod->mod_end = P2V(mod->mod_end);
			 mod->cmdline = P2V(mod->cmdline);
			 mod++;
		}
	}

//	serial_printf("TETST %x\n", P2V(mb->mmap_addr));	
	vbe_info = (struct vbe_mode_info *)P2V(mb->vbe_mode_info);
	info->framebuffer = (void *)vbe_info->framebuffer_address; 
	serial_printf("MMM %x\n", info->framebuffer);
	info->x_res = vbe_info->x_res;
	info->y_res = vbe_info->y_res;
	info->bpp = vbe_info->bpp;
	info->mode = mb->vbe_mode;

	return info;
}

void modules_init(struct multiboot_info *mb)
{
	extern uint32_t *background_image;
	//kernel_commandline = (char *)(mb->boot_device);
	if(mb->mods_count > 0 )
	{
		background_image  = (uint32_t *)P2V(*((void**)P2V(mb->mods_addr)));
	//	uintptr_t end  = (uintptr_t)*((void **)P2V(mb->mods_addr + 4));
	//	serial_printf("END %X\n", end);
	//	initrd_init(P2V(start),P2V(end));
	}
}

/*
void kmain_multiboot(struct multiboot_info * mb UNUSED, uint32_t magic)
{
	//init polling serial here for printf_serial
	//
 	if ( magic != 0x2BADB002 )
   	{
	//	console_puts("Bad magic number, halting\r");
		return;
   	}
	while(1);
	//have this parse the multiboot header into a COS
	//specific struct format
	//struct kernel_info *info = multiboot_parse(mb);
	//modules_init(mb);
*/
/*extern uint32_t *MultiBootHeader;

	printf("MUT %x\n", MultiBootHeader);
   struct multiboot_header
     {
        Must be MULTIBOOT_MAGIC - see above. 
       multiboot_uint32_t magic;
     
       multiboot_uint32_t flags;
     
       multiboot_uint32_t checksum;
     
       multiboot_uint32_t header_addr;
       multiboot_uint32_t load_addr;
       multiboot_uint32_t load_end_addr;
       multiboot_uint32_t bss_end_addr;
       multiboot_uint32_t entry_addr;


struct multiboot_header *mhdr = (void *)&MultiBootHeader;
	printf("%x %x %x %x %x\n", mhdr->header_addr, mhdr->load_addr, mhdr->load_end_addr, mhdr->bss_end_addr, mhdr->entry_addr);
extern void multiboot_print(struct multiboot_info *mb);
	multiboot_print(mb);
	while(1);*/

	//kmain(info);
/*

}
*/

//FIXME: Move this somewhere else (init/boot.c?)
void print_mb(unsigned long addr, unsigned long magic);

void print_mb(unsigned long addr, unsigned long magic);
//void multiboot_print(struct multiboot_info *mb)
//{
/*	struct vbe_mode_info {
		char signature[4];
		short version;
		short oemstr[2];
		unsigned char capabilities[4];
		short videomodes[2];
		short totalmem;
		short soft_ver;
		short vendstr[2];
		uint32_t prodstr;
		uint32_t prodrevstr;
	
	} *vbe_info = kcalloc(sizeof(*vbe_info), 1); 
	struct ModeInfoBlock {
		uint16_t attributes;
		uint8_t winA,winB;
		uint16_t granularity;
		uint16_t winsize;
		uint16_t segmentA, segmentB;
		uintptr_t realFctPtr;
		uint16_t pitch; // chars per scanline
			   
		uint16_t Xres, Yres;
		uint8_t Wchar, Ychar, planes, bpp, banks;
		uint8_t memory_model, bank_size, image_pages;
		uint8_t reserved0;
		
		uint8_t red_mask, red_position;
		uint8_t green_mask, green_position;
		uint8_t blue_mask, blue_position;
		uint8_t rsv_mask, rsv_position;
		uint8_t directcolor_attributes;
		
		uint32_t physbase;  // your LFB (Linear Framebuffer) address ;)
		uint32_t reserved1;
		short reserved2;
	} *mode_info;
	mode_info = (void *)P2V(mb->vbe_mode_info);

	printf("TEST %x\n", mode_info->physbase);
//	thread_t *cur = thread_current();
	extern uint32_t * kernel_pd;
	uintptr_t framebuffer = mode_info->physbase;
	uint32_t size = mode_info->Xres * mode_info->Yres * mode_info->bpp;
	printf("X %i Y %i bpp %i SIZE %i\n", mode_info->Xres , mode_info->Yres , mode_info->bpp, size);
	pagedir_insert_pagen_physical(kernel_pd, (uintptr_t)framebuffer, (uintptr_t)framebuffer, 0x7, size/4096);
	pagedir_install(kernel_pd);
	kmemset((void *)framebuffer, 0xAA,size);
	kmemcpy(vbe_info, (void *)P2V(mb->vbe_control_info), sizeof(*vbe_info));
	void *oemstr = (void *)*(uint32_t *)&vbe_info->oemstr;
	printf("Sig %4s\n", vbe_info->signature);
 	printf("Sig %x\n", vbe_info->version); 
 	printf("string %x %x\n", vbe_info->oemstr[0],vbe_info->oemstr[1]); 
	printf("Strinf %x\n", (uint32_t *)oemstr);
 	printf("string %x %x\n", vbe_info->vendstr[0],vbe_info->vendstr[1]); 
 	printf("Pointer? %.4x%.4x\n", vbe_info->videomodes[0] & 0xFFFF,vbe_info->videomodes[1] & 0xFFFF);*/ 
/*printf("Control info %x  mode info %x mode %x seg %x off %x len %x\n", mb->vbe_control_info,
        mb->vbe_mode_info,
        mb->vbe_mode,
        mb->vbe_interface_seg,
        mb->vbe_interface_off,
        mb->vbe_interface_len);*/

//	print_mb((unsigned long)mb, 0x2BADB002);

//}
//this needs to go somewhere else
void print_mb(unsigned long addr, unsigned long magic)
{
     #define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
	//addr = P2V(addr);
 /* Am I booted by a Multiboot-compliant boot loader? */
       if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
         {
           printf ("Invalid magic number: 0x%x\n", (unsigned) magic);
           return;
         }
           multiboot_info_t *mbi;
 
       /* Set MBI to the address of the Multiboot information structure. */
       mbi = (multiboot_info_t *) addr;
     
       /* Print out the flags. */
       printf ("flags = 0x%x\n", (unsigned) mbi->flags);
     
       /* Are mem_* valid? */
       if (CHECK_FLAG (mbi->flags, 0))
         printf ("mem_lower = %iKB, mem_upper = %iKB\n",
                 (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
     
       /* Is boot_device valid? */
       if (CHECK_FLAG (mbi->flags, 1))
         printf ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
     
       /* Is the command line passed? */
       if (CHECK_FLAG (mbi->flags, 2))
         printf ("cmdline = %s\n", (char *) P2V(mbi->cmdline));
     
       /* Are mods_* valid? */
       if (CHECK_FLAG (mbi->flags, 3))
         {
           multiboot_module_t *mod;
           unsigned int i;
     
           printf ("mods_count = %d, mods_addr = 0x%x\n",
                   (int) mbi->mods_count, (int) mbi->mods_addr);
           for (i = 0, mod = (multiboot_module_t *) P2V(mbi->mods_addr);
                i < mbi->mods_count;
                i++, mod++)
             printf (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %p\n",
                     (unsigned) P2V(mod->mod_start),
                     (unsigned) P2V(mod->mod_end),
                     (char *) P2V(mod->cmdline));
         }
     
       /* Bits 4 and 5 are mutually exclusive! */
       if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
         {
           printf ("Both bits 4 and 5 are set.\n");
           return;
         }
     //return;
       /* Is the symbol table of a.out valid? */
       if (CHECK_FLAG (mbi->flags, 4))
         {
           multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);
     
           printf ("multiboot_aout_symbol_table: tabsize = 0x%0x, "
                   "strsize = 0x%x, addr = 0x%x\n",
                   (unsigned) multiboot_aout_sym->tabsize,
                   (unsigned) multiboot_aout_sym->strsize,
                   (unsigned) multiboot_aout_sym->addr);
         }
    	 
       /* Is the section header table of ELF valid? */
       if (CHECK_FLAG (mbi->flags, 5))
         {
           multiboot_elf_section_header_table_t *multiboot_elf_sec = (&(mbi->u.elf_sec));
     
           printf ("multiboot_elf_sec: num = %u, size = 0x%x,"
                   " addr = 0x%x, shndx = 0x%x\n",
                   (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
                   (unsigned) P2V(multiboot_elf_sec->addr), (unsigned) multiboot_elf_sec->shndx);
         }
//return;
      /* Are mmap_* valid? */
       if (CHECK_FLAG (mbi->flags, 6))
         {
           multiboot_memory_map_t *mmap;
     
           printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
                   (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
           for (mmap = (multiboot_memory_map_t *) P2V(mbi->mmap_addr);
                (unsigned long) mmap < P2V(mbi->mmap_addr + mbi->mmap_length);
                mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                         + mmap->size + sizeof (mmap->size)))
             printf (" size = 0x%x, base_addr = 0x%x,"
                     " length = 0x%x, type = 0x%x\n",
                     (unsigned) mmap->size,
                     (unsigned) mmap->addr,
                     (unsigned) mmap->len,
                     (unsigned) mmap->type);
         
		} 

}

