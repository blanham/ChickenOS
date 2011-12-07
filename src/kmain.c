#include <kernel/console.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/timer.h>
#include <kernel/types.h>
#include <kernel/vm.h>
#include <../fs/vfs.h>
#include <../fs/initrd.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "syscall.h"

extern void context_switch();
void print_mb(unsigned long addr, unsigned long magic);
//#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
char *msg = "HERPDERP";
void idle(void *aux)
{
	aux = aux;
	while(1)
		asm volatile("hlt");
}

//void console_clear()
void dummy(void *aux)
{
	aux = aux;

	char buf[256];
	char out[256];

	kmemset((uint8_t *)out, 0, 255);
	//for(int i = 0; i < 24; i++)
	//	console_puts("p\n");
	char c;
	int fd = open((char*)aux, 0);
//	printf("fd = %i\n",fd);
	kmemset((uint8_t *)buf, 0 , 255);
	size_t ret = read(fd, buf, 10);
	//printf("read %x\n",ret);
//	sprintf(out, "TEST %s\n",buf);
	//sprintf(out, "read %x\n", ret);

//	write(fd, "DERPppppTT", ret);
	
//	int fd2 = open("/dev/initrd",0);
//	printf("FD %i\n", fd2);
//	kmemset((uint8_t *)buf, 0, 256);

//	ret = read(fd2, buf, 32);
//	ret= ret;
//	printf("RET %s\n",buf);
	write(fd, buf, ret);
	printf("done\n");
	while(1);
	while(1)
	{
		c = getchar();
		putc(c);

	}
	while(1)
	{
		printf("TEST %i %s\n",thread_current()->pid, (char *)aux);
		thread_yield();

	}
	//gets(test);
//	puts(test);
//	printf("%s\n",test);
//	int t = strcmp(test, "test");
//	printf("%s\n",test);
//	t =t;
//	if(t == -1)
//		printf("YES\n");
//	else
//		printf("NO\n");
//	while(1);
		printf("%s\n",(uint8_t *)aux);
	thread_exit();
//	while(1);
}

extern int sh_main(void *aux);

void init(void *aux)
{
	int ret = sh_main(aux);
	printf("return %i\n",ret);
	thread_exit();
}

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
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
		puts("Bad magic number, halting\r");
		return;
   	}
 	mb = (struct multiboot_info *)P2V(mbd);
	/* begin initializations */
	interrupt_init();
	vm_init(mb);
	console_init();
	paging_init();
	console_set_color(BLUE,WHITE);
	console_puts("ChickenOS v0.01 booting\n");
	
	time_init();
	syscall_init();
	thread_init();
	asm volatile("sti");	
	
	vfs_init();
//	modules_init(mb);	
	console_fs_init();
	console_set_color(BLACK,WHITE);
	thread_create(idle,NULL);
	//thread_create(dummy,"/dev/tty");
	thread_create(dummy,"/dev/tty0");
	thread_create(init,NULL);
//	thread_create(dummy,"TEST2");
	//uint8_t*g = NULL;
	//*g = 9;
	thread_exit();
	
	PANIC("kmain returned");
}
void print_mb(unsigned long addr, unsigned long magic)
{
     #define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
	addr = P2V(addr);
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
             printf (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
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
     return;
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

