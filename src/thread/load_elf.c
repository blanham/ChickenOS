#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <mm/paging.h>
#include <mm/liballoc.h>
#include <thread.h>
#include <fs/vfs.h>
#define PT_LOAD 0x1
typedef struct elf_header {
	uint8_t magic[4];
	uint8_t class;
	uint8_t byteorder;
	uint8_t hversion;
	uint8_t pad[9];
	uint16_t filetype;
	uint16_t archtype;
	uint32_t fversion;
	uint32_t entry;
	uint32_t phdrpos;
	uint32_t shdrpos;
	uint32_t flags;
	uint16_t hdrsize;
	uint16_t phdrent;
	uint16_t phdrcnt;
	uint16_t shdrent;
	uint16_t shdrcnt;
	uint16_t strsec;
} __attribute__((packed)) elf_header_t;
typedef struct elf_section {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_align;
	uint32_t sh_entsize;
}  __attribute__((packed)) elf_section_t;
typedef struct elf_program_header {
	uint32_t ph_type;
	uint32_t ph_offset;
	uint32_t ph_virtaddr;
	uint32_t ph_physaddr;
	uint32_t ph_filesize;
	uint32_t ph_memsize;
	uint32_t ph_flags;
	uint32_t ph_align;
} __attribute__((packed)) elf_program_header_t;
//	PRIntf("phdrpos %X shdrpos %X\n",header->phdrpos, header->shdrpos);
//	printf("phdrent %X phdrcnt %i shdrent %X shdrcnt %i\n",	
//		header->phdrent, header->phdrcnt, header->shdrent, header->shdrcnt);

void elf_print_sections(elf_section_t *sections)
{
//	uint32_t sh_name;
//	uint32_t sh_type;
//	uint32_t sh_flags;
	printf("type %i addr %X offset %i size %X link %X info %X align %X entsize %x\n",
 		sections->sh_type, sections->sh_addr, sections->sh_offset, sections->sh_size,
		sections->sh_link, sections->sh_info, sections->sh_align, sections->sh_entsize);

}

void elf_print_programs(elf_program_header_t *program)
{
	printf("type %i offset %X virtaddt %X filesize %X memsize %X align %X\n",
		program->ph_type, program->ph_offset, program->ph_virtaddr,
 		program->ph_filesize, program->ph_memsize, program->ph_align);
}

static void elf_load_program(elf_header_t *header, int fd)
{
	elf_program_header_t *program = kmalloc(sizeof(*program)*header->phdrcnt);
	void *code;
	pagedir_t pd = thread_current()->pd;
	int pages = 0;
	sys_lseek(fd, header->phdrpos, SEEK_SET);
	sys_read(fd, program, sizeof(*program)*header->phdrcnt);
	for(int i = 0; i < header->phdrcnt; i++)
	{
		if(program->ph_type == PT_LOAD)
		{
		//	elf_print_programs(program);
			sys_lseek(fd, program->ph_offset, SEEK_SET);
			pages = program->ph_memsize/PAGE_SIZE + 1;
			code = pallocn(pages);
			
			if(program->ph_filesize < program->ph_memsize)
			{
				memset((void *)(code + program->ph_filesize), 0x0, program->ph_memsize - program->ph_filesize);
			}
			if((program->ph_offset & 0xFFF) != 0)
			{
				code = code + (program->ph_offset & 0xfff) ;
			}
			sys_read(fd, code, program->ph_filesize);

			pagedir_insert_pagen(pd, (uintptr_t)((uintptr_t) code & ~0xFFF), program->ph_virtaddr, 0x7, pages);
		//	printf("insert %i pages\n",pages);
		}	
		program++;
	}

	pagedir_install(pd);
}
int load_elf(const char *path, uintptr_t *eip)
{
	int fd;
	elf_header_t *header;
	header = kmalloc(sizeof(*header));

	if((fd = sys_open(path, 0)) < 0)
	{
		printf("wellp, we're boned\n");
		return -1;
	}	
		
	if(sys_read(fd, header, sizeof(*header)) != sizeof(*header))
	{
		printf("Error reading ELF header\n");
		return -1;
	}
	if(memcmp(header->magic, ELF_MAGIC, 4) != 0)
	{
		printf("Missing or invalid ELF magic number!\n");
		return -1;
	}
	
	elf_load_program(header, fd);	
	
	*eip = header->entry;
	kfree(header);
	sys_close(fd);
	
	return 0;
}

