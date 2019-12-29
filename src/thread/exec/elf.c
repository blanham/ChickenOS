#include <common.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>
#include <chicken/thread.h>
#include <chicken/thread/exec.h>
#include <mm/vm.h>
#include <mm/liballoc.h>
#include <errno.h>
#include <fs/vfs.h>

void elf_map_regions(Elf32_Ehdr *header, struct file *file)
{
	thread_t *cur = thread_current();
	Elf32_Phdr *phdr = kcalloc(sizeof(*phdr), header->e_phnum);

	//NO //XXX: Why did I say no?
	file->offset = header->e_phoff;
	vfs_read(file, phdr, sizeof(*phdr)*header->e_phnum);

	for(uint16_t i = 0; i < header->e_phnum; i++, phdr++)
	{
		if(phdr->p_type != PT_LOAD)
			continue;

		//elf_print_programs(phdr);

		//memregion_add(cur->mm, phdr->p_vaddr, phdr->p_memsz, PROT_READ | PROT_EXEC,
		//		MAP_FILE, file->inode, phdr->p_offset, phdr->p_filesz, NULL);

		uint32_t mem_length = (phdr->p_vaddr & ~PAGE_MASK) + phdr->p_memsz;
		uint32_t file_length = (phdr->p_offset & ~PAGE_MASK) + phdr->p_filesz;
		memregion_map_file(cur->mm, phdr->p_vaddr, mem_length, PROT_READ|PROT_WRITE|PROT_EXEC,
				MAP_FILE, file->inode, phdr->p_offset & PAGE_MASK, file_length);
		serial_printf("Vaddr %8x Filesize %x Memsize %x Offset %x\n", phdr->p_vaddr,
				phdr->p_filesz, phdr->p_memsz, phdr->p_offset);
	}
}

int load_elf(executable_t *exe)
{
	int ret = 0;

	struct file *file = exe->file;
	// XXX: Fix this too
	file->offset = 0;

	Elf32_Ehdr *header = kmalloc(sizeof(*header));
	if ((ret = vfs_read(file, header, sizeof(*header))) != sizeof(*header)) {
		printf("Error reading ELF header\nShould have read %x bytes, actually read %x bytes\n",
				sizeof(*header), ret);
		return -1;
	}
	if (memcmp(header->e_ident, ELFMAG, SELFMAG)) {
		printf("Missing or invalid ELF magic number!\n");
		return -1;
	}
	//printf("HERE!!!");

	elf_map_regions(header, file);
	exe->ip = header->e_entry;

	kfree(header);

	return 0;
}


