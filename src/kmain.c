#include <kernel/types.h>
#include <kernel/interrupt.h>
#include "syscall.h"
#include <stdio.h>
#include <kernel/console.h>
#include <kernel/timer.h>
#include <kernel/vm.h>
#include <kernel/hw.h>
#include <kernel/thread.h>
#include <multiboot.h>
#include "debug.h"

extern void context_switch();
void print_mb(unsigned long addr, unsigned long magic);
//#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

void idle(void *aux)
{
	asm volatile("sti");
	aux = aux;
	printf("awesome\n");
	while(1);
}
void kmain(uint32_t mbd, uint32_t magic)
{
	struct multiboot_info *mb;
   	if ( magic != 0x2BADB002 )
   	{
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
		puts("Bad magic number, halting\r");
		return;
   	}
 	mb = (struct multiboot_info *)mbd;
	
	/* begin initializations */
	paging_init();	
	interrupt_init();
	console_init();
	console_set_color(BLUE,WHITE);
	time_init();
/*	if(mb->mods_count > 0 )
	{
		printf("Modules %i\n", mb->mods_count);
		void *ptr = *(void **)mb->mods_addr; 
		puts((char *)ptr);
		puts("\n");
		printf("addr %x\n", *(void **)(mb->mods_addr + 4));
		printf("addr %x\n", *(void **)(mb->mods_addr));
	//	initrd_init(*(void**)mb->mods_addr);
		placement = (unsigned)*(void **)(mb->mods_addr + 4);	
	}
*/

	console_puts("ChickenOS v0.01 booting\n");
	vm_init(mb->mem_upper);
	syscall_init();//registers interrupt handler
	console_set_color(BLACK,WHITE);
	
	thread_init();
	asm volatile("sti");	
	printf("hello, world \n");
	//sys_dummy();
//	char *p = (char *)0x8000000;
//	*p = *p;
	//asm volatile ( "mov $0,%%eip");	
//	context_switch();
//	thread_t *cur = thread_current();
//	printf("pid %i\n",cur->pid);
	printf("IDLE %X %X\n",idle, &idle);
	thread_create(idle,NULL);
	//idle(NULL);
	printf("TEST\n");
	while(1);
	printf("DERP\n");
	//	printf("kmain\n");
	PANIC("kmain returned");
}
void print_mb(unsigned long addr, unsigned long magic)
{
     #define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

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
         printf ("mem_lower = %xKB, mem_upper = %xKB\n",
                 (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
     
       /* Is boot_device valid? */
       if (CHECK_FLAG (mbi->flags, 1))
         printf ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
     
       /* Is the command line passed? */
       if (CHECK_FLAG (mbi->flags, 2))
         printf ("cmdline = %s\n", (char *) mbi->cmdline);
     
       /* Are mods_* valid? */
       if (CHECK_FLAG (mbi->flags, 3))
         {
           multiboot_module_t *mod;
           unsigned int i;
     
           printf ("mods_count = %d, mods_addr = 0x%x\n",
                   (int) mbi->mods_count, (int) mbi->mods_addr);
           for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
                i < mbi->mods_count;
                i++, mod++)
             printf (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
                     (unsigned) mod->mod_start,
                     (unsigned) mod->mod_end,
                     (char *) mod->cmdline);
         }
     
       /* Bits 4 and 5 are mutually exclusive! */
       if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
         {
           printf ("Both bits 4 and 5 are set.\n");
           return;
         }
     
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
           multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);
     
           printf ("multiboot_elf_sec: num = %u, size = 0x%x,"
                   " addr = 0x%x, shndx = 0x%x\n",
                   (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
                   (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
         }
      /* Are mmap_* valid? */
       if (CHECK_FLAG (mbi->flags, 6))
         {
           multiboot_memory_map_t *mmap;
     
           printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
                   (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
           for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
                (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                         + mmap->size + sizeof (mmap->size)))
             printf (" size = 0x%x, base_addr = 0x%x%x,"
                     " length = 0x%x%x, type = 0x%x\n",
                     (unsigned) mmap->size,
                     (unsigned) mmap->base_addr_high,
                     (unsigned) mmap->base_addr_low,
                     (unsigned) mmap->length_high,
                     (unsigned) mmap->length_low,
                     (unsigned) mmap->type);
         } 

}

