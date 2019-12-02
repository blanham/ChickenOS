#include <stdio.h>
#include <string.h>
#include <chicken/thread/exec.h>

bool elf_check_magic(void *magic)
{
	return memcmp(magic, ELFMAG, SELFMAG) == 0;
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