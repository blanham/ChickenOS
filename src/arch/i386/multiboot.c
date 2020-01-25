/*	ChickenOS - arch/i386/multiboot.c - multiboot accesory functions
 *
 *	This file takes the multiboot structs and uses them to fill
 *	out a struct kernel_boot_info
 */
#include <string.h>
#include <multiboot.h>
#include <chicken/boot.h>
#include <chicken/common.h>
#include <chicken/thread.h>
#include <chicken/vbe.h>

#define BOOT_DEBUG

struct kernel_boot_info multiboot_storage;
char multiboot_cmdline[256];

//TODO: For now we only support 4 modules
#define MAX_MODS_COUNT 4
struct boot_module modules[MAX_MODS_COUNT];

extern uintptr_t end;

static void multiboot_detect_video_mode(struct multiboot_info *mb, struct kernel_boot_info *info)
{
	info->x_chars = 80;
	info->y_chars = 25;
	if (mb->framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT) {
		info->x_res = 80;
		info->y_res = 25;
	} else {
		struct vbe_mode_info *vbe_info = (struct vbe_mode_info *)P2V(mb->vbe_mode_info);
		info->mode = mb->vbe_mode;
		info->framebuffer = (void *)P2V(vbe_info->framebuffer_address);
#ifdef BOOT_DEBUG
		serial_printf("Framebuffer %x\n", info->framebuffer);
#endif
		info->x_res = vbe_info->x_res;
		info->y_res = vbe_info->y_res;
		info->bpp = vbe_info->bpp;
	}
}

static uint32_t multiboot_parse_modules(struct multiboot_info *mb, struct kernel_boot_info *info)
{
	uint32_t last_address = 0;
	//extern uint32_t *background_image;

	if (!(mb->flags & MULTIBOOT_INFO_MODS))
		goto no_modules;

	if (mb->mods_count == 0)
		goto no_modules;

	int mods_count = mb->mods_count;
	if (mods_count > MAX_MODS_COUNT)
		mods_count = MAX_MODS_COUNT;

	multiboot_module_t *mb_mods = (multiboot_module_t *) P2V(mb->mods_addr);

	for (int i = 0; i < mods_count; i++) {
		modules[i].start = (uintptr_t)P2V(mb_mods->mod_start);
		modules[i].end = (uintptr_t)P2V(mb_mods->mod_end);
		if (modules[i].end > last_address)
			last_address = modules[i].end;
		void *mod_cmdline = (void *)P2V(mb_mods->cmdline);
		memcpy(modules[i].cmdline, mod_cmdline, strlen(mod_cmdline));
		mb_mods++;
	}

	info->modules = modules;
	info->modules_count = mods_count;

no_modules:
	return last_address;
}

//FIXME: This should be cleaned up
static uint32_t multiboot_parse_memmap(struct multiboot_info *mb)
{
	multiboot_memory_map_t *mmap = (void *)P2V(mb->mmap_addr);
	void *mmap_end = P2V(mb->mmap_addr + mb->mmap_length);
	uint32_t last_address = 0;
	uint32_t mem_size = 0;
#ifdef BOOT_DEBUG
		serial_printf (" size = 0x%x, base_addr = 0x%x, length = 0x%x, type = 0x%x\n",
				(unsigned)mmap->size, (unsigned)mmap->addr,
				(unsigned)mmap->len, (unsigned)mmap->type);
#endif
	while((uintptr_t)mmap < (uintptr_t)mmap_end) {
		mmap = (void *)((uintptr_t)mmap +mmap->size + sizeof(mmap->size));
#ifdef BOOT_DEBUG
		serial_printf (" size = 0x%x, base_addr = 0x%x, length = 0x%x, type = 0x%x\n",
				(unsigned)mmap->size, (unsigned)mmap->addr,
				(unsigned)mmap->len, (unsigned)mmap->type);
#endif
		if (!(mmap->type & MULTIBOOT_MEMORY_AVAILABLE))
			continue;

		//TODO: Have the memory manager code look through a memory map
		//		from kernel_boot_info
		// Find the largest block, that will be the end of main memory for now
		if (mmap->len > mem_size) {
			mem_size = mmap->len;
			last_address = mmap->addr + mmap->len;
		}
	}

	return mem_size;
}

struct kernel_boot_info *multiboot_parse(struct multiboot_info *mb, uint32_t magic)
{
	struct kernel_boot_info *info = &multiboot_storage;
	uintptr_t first_available_page = 0;
	uintptr_t placement = (uintptr_t )&end;

	if(magic != MULTIBOOT_BOOTLOADER_MAGIC)
		return NULL;

#ifdef BOOT_DEBUG
	extern void serial_init();
	serial_init();
#endif
	serial_printf("placement: %X end: %x\n", placement, end);

	memcpy(multiboot_cmdline, (void *)P2V(mb->cmdline), strlen((char *)P2V(mb->cmdline)));
	info->cmdline = multiboot_cmdline;

	// Keep a reference to the multiboot info just in case
	info->aux = mb;

	//HACK: Apparently all of the info stuff is loaded last
	//      so without this we end up trashing it
	first_available_page = (uintptr_t)P2V((mb->vbe_mode_info + PAGE_SIZE) & PAGE_MASK);

	uint32_t last_module_address = multiboot_parse_modules(mb, info);

	uint32_t last_available_page = multiboot_parse_memmap(mb);

	uintptr_t last_mb_data = ((uintptr_t)&mb->vbe_mode & ~0xFFF) + 4096*8;
	serial_printf("md: %X\n", last_mb_data);

	// Use the highest address as our placement
	if(last_mb_data > first_available_page)
		first_available_page = last_mb_data;
	if(first_available_page > placement)
		placement = first_available_page;
	if(last_module_address > placement)
		placement = last_module_address;

	placement = (placement & ~0xFFF) + PAGE_SIZE;

	info->placement = (void *)placement;
	//FIXME: This isn't right, but it is less wrong than before :P
	(void)last_available_page;
	info->mem_size = last_available_page ;
	info->low_mem = mb->mem_lower;
	info->hi_mem = mb->mem_upper;

	serial_printf("placement: %X end: %x\n", placement, &end);
	multiboot_detect_video_mode(mb, info);

	return info;
}

