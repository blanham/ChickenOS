
//this needs to go somewhere else
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
return;
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

