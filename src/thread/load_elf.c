#include <common.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>
#include <mm/vm.h>
#include <mm/liballoc.h>
#include <errno.h>
#include <thread.h>
#include <fs/vfs.h>
#include <elf.h>

#define ELF_MIN_ALIGN PAGE_SIZE
#define ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(ELF_MIN_ALIGN-1))
#define ELF_PAGEOFFSET(_v) ((_v) & (ELF_MIN_ALIGN-1))
#define ELF_PAGEALIGN(_v) (((_v) + ELF_MIN_ALIGN - 1) & ~(ELF_MIN_ALIGN - 1))

bool elf_check_magic(void *magic)
{
	if(memcmp(magic, ELFMAG, SELFMAG) == 0)
		return true;

	return false;
}

void elf_print_sections(Elf32_Shdr  *sections)
{
	printf("type %i addr %X offset %i size %X link %X info %X align %X entsize %x\n",
			sections->sh_type, sections->sh_addr, sections->sh_offset, sections->sh_size,
			sections->sh_link, sections->sh_info, sections->sh_addralign, sections->sh_entsize);
}

void elf_print_programs(Elf32_Phdr *program)
{
	printf("type %i offset %X virtaddt %X filesize %X memsize %X align %X\n",
			program->p_type, program->p_offset, program->p_vaddr,
			program->p_filesz, program->p_memsz, program->p_align);
}

void elf_map_regions(Elf32_Ehdr *header, struct file *file)
{
	thread_t *cur = thread_current();
	Elf32_Phdr *phdr = kcalloc(sizeof(*phdr), header->e_phnum);

	//NO
	file->offset = header->e_phoff;
	vfs_read(file, phdr, sizeof(*phdr)*header->e_phnum);

	for(uint16_t i = 0; i < header->e_phnum; i++, phdr++)
	{
		if(phdr->p_type != PT_LOAD)
			continue;

		elf_print_programs(phdr);

		//memregion_add(cur->mm, phdr->p_vaddr, phdr->p_memsz, PROT_READ | PROT_EXEC,
		//		MAP_FILE, file->inode, phdr->p_offset, phdr->p_filesz, NULL);

		memregion_map_file(cur->mm, phdr->p_vaddr, phdr->p_memsz, PROT_READ | PROT_EXEC,
				MAP_FILE, file->inode, phdr->p_offset, phdr->p_filesz);
		printf("Vaddr %8x Filesize %x Memsize %x Offset %x\n", phdr->p_vaddr,
				phdr->p_filesz, phdr->p_memsz, phdr->p_offset);
	}
}

int load_elf(const char *path, uintptr_t *eip)
{
	Elf32_Ehdr *header = kmalloc(sizeof(*header));
	struct file *file;
	int ret = 0;

	file = vfs_open((char *)path, 0, 0);
	if(file == NULL)
	{
		printf("Failed to open elf executable\n");
		return -1;
	}

	if((ret = vfs_read(file, header, sizeof(*header))) != sizeof(*header))
	{
		printf("Error reading ELF header\nShould have read %x bytes, actually read %x bytes\n",
				sizeof(*header), ret);
		return -1;
	}
	if(memcmp(header->e_ident, ELFMAG, SELFMAG) != 0)
	{
		printf("Missing or invalid ELF magic number!\n");
		return -1;
	}

	elf_map_regions(header, file);
	*eip = header->e_entry;

	kfree(header);

	return 0;
}


