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

void elf_load_program(Elf32_Ehdr *header, struct file *file)
{
	Elf32_Phdr *program;
	pagedir_t pd = thread_current()->mm->pd;
	void *code;
	int pages = 0;

   	program = kcalloc(sizeof(*program), header->e_phnum);

	//FIXME: hack so we don't allocate fd
	file->offset = header->e_phoff;
	vfs_read(file, program, sizeof(*program)*header->e_phnum);

	for(int i = 0; i < header->e_phnum; i++)
	{
		if(program->p_type == PT_LOAD)
		{
			//elf_print_programs(program);
			file->offset =program->p_offset;
			pages = program->p_memsz/PAGE_SIZE + 1;

			code = pallocn(pages);

			void *old = code;	
			if(program->p_filesz < program->p_memsz)
			{
				memset(code, 0, pages * PAGE_SIZE);
				//memset((void *)(code + program->ph_filesize), 0x0, program->ph_memsize - program->ph_filesize);
			}
			if((program->p_offset & 0xFFF) != 0)
			{
				code = code + (program->p_offset & 0xfff) ;
				//program->ph_virtaddr -= (program->ph_offset & 0xfff);
				pages++;
				//printf("code %p virtaddr %p\n", code,program->ph_virtaddr);
			}
			vfs_read(file, code, program->p_filesz);
		//	printf("%p %p %p %p %i %i\n",
		//			code,old, program->ph_virtaddr, program->ph_offset, 
		//			program->ph_memsize/PAGE_SIZE,pages); 
			pagedir_insert_pagen(pd, (uintptr_t)old, program->p_vaddr, 0x7, pages+1);
		//	printf("insert %i pages\n",pages);
		}	
		program++;
	}

	pagedir_install(pd);
}
#define ELF_MIN_ALIGN PAGE_SIZE
#define ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(ELF_MIN_ALIGN-1))
#define ELF_PAGEOFFSET(_v) ((_v) & (ELF_MIN_ALIGN-1))
#define ELF_PAGEALIGN(_v) (((_v) + ELF_MIN_ALIGN - 1) & ~(ELF_MIN_ALIGN - 1))

void elf_load_program2(Elf32_Ehdr *header, struct file *file)
{
	Elf32_Phdr *phdr;
	thread_t *cur = thread_current();

   	phdr = kcalloc(sizeof(*phdr), header->e_phnum);

	file->offset = header->e_phoff;

	printf("OFFSET %x\n", file->offset);

	vfs_read(file, phdr, sizeof(*phdr)*header->e_phnum);

	for(uint16_t i = 0; i < header->e_phnum; i++)
	{
		if(phdr->p_type != PT_LOAD)
			continue;

		elf_print_programs(phdr);

		printf("Mem %x \n", phdr->p_memsz + phdr->p_vaddr);

//			   eppnt->p_offset - ELF_PAGEOFFSET(eppnt->p_vaddr));
	//	uintptr_t dif = 0;
	//	if((phdr->p_vaddr & 0xFFF) != 0)
	//	{
	//		dif = phdr->p_vaddr & 0xfff;
	//		phdr->p_vaddr &= ~0xFFF;
	//		//phdr->p_vaddr += 0x1000;
	//	}
	//	if((phdr->p_memsz & 0xFFF) != 0)
	//	{
	//		printf("OLD MEMSIZE %x\n", phdr->p_memsz);
	//		phdr->p_memsz &= ~0xFFF;
	//		phdr->p_memsz += 0x1000;
	//	}
	//	if((phdr->p_offset & 0xFFF) != 0)
	//	{
	//	//	phdr->p_offset &= ~0xFFF;
	//	//	phdr->p_offset += 0x1000;
	//	}

	//	file->offset = phdr->p_offset;
	//	//file->offset = phdr->p_offset- ELF_PAGEOFFSET(phdr->p_vaddr);

	//	//memregion_add(cur, phdr->p_vaddr, 
	//	//		phdr->p_memsz, 0, 0, file, NULL);
	//	void *data = pallocn((phdr->p_memsz / PAGE_SIZE) + 1);
	//	data += dif;
	////	vfs_read(file, data, phdr->p_filesz);
	//	printf("DATA %X %X %X\n", ((int *)data)[0],   ((int *)data)[1], ((int *)data)[2] );
		uint32_t offset = phdr->p_offset & ~0xFFF;
		uint32_t vaddr = phdr->p_vaddr & ~0xFFF;
		uint32_t memsz = phdr->p_memsz;
		file->offset = offset;
		//if(vaddr == 0x806a000)
		//	memsz += 0x2000;
		memregion_add(cur, vaddr, memsz, 0, 0, file, NULL);
		

	//	memregion_add(cur, phdr->p_vaddr, phdr->p_memsz + 0x1000, 0, 0, file, NULL);
		printf("Vaddr %8x Memsize %x Offset %x\n", phdr->p_vaddr, phdr->p_memsz, phdr->p_offset); 
		(void)cur;

		phdr++;
	}
}

void elf_load_program3(Elf32_Ehdr *header, struct file *file)
{
	Elf32_Phdr *program;
	pagedir_t pd = thread_current()->mm->pd;
	void *code;
	int pages = 0;
	thread_t *cur = thread_current();

   	program = kcalloc(sizeof(*program), header->e_phnum);

	//FIXME: hack so we don't allocate fd
	file->offset = header->e_phoff;
	vfs_read(file, program, sizeof(*program)*header->e_phnum);

	for(int i = 0; i < header->e_phnum; i++)
	{
		if(program->p_type == PT_LOAD)
		{
			//elf_print_programs(program);
			file->offset =program->p_offset;
			pages = program->p_memsz/PAGE_SIZE + 1;

			code = pallocn(pages);

			void *old = code;	
			if(program->p_filesz < program->p_memsz)
			{
				memset(code, 0, pages * PAGE_SIZE);
				//memset((void *)(code + program->ph_filesize), 0x0, program->ph_memsize - program->ph_filesize);
			}
			if((program->p_offset & 0xFFF) != 0)
			{
				code = code + (program->p_offset & 0xfff) ;
				//program->ph_virtaddr -= (program->ph_offset & 0xfff);
				pages++;
				//printf("code %p virtaddr %p\n", code,program->ph_virtaddr);
			}
			vfs_read(file, code, program->p_filesz);
		//	printf("%p %p %p %p %i %i\n",
		//			code,old, program->ph_virtaddr, program->ph_offset, 
		//			program->ph_memsize/PAGE_SIZE,pages); 
			//pagedir_insert_pagen(pd, (uintptr_t)old, program->p_vaddr, 0x7, pages+1);
			memregion_add(cur, program->p_vaddr, 
				program->p_memsz + 0x1000, 0, 0, NULL, old);


		//	printf("insert %i pages\n",pages);
		}	
		program++;
	}

	pagedir_install(pd);
}

int load_elf(const char *path, uintptr_t *eip)
{
	struct file *file;
	int ret = 0;
	Elf32_Ehdr *header;
	header = kmalloc(sizeof(*header));
	
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
	
//	elf_load_program(header, file);	
	//elf_load_program2(header, file);	
	elf_load_program2(header, file);	
//	while(1);	
	*eip = header->e_entry;
	kfree(header);
//	sys_close(fd);
	
	return 0;
}

bool elf_check_magic(void *magic)
{
	if(memcmp(magic, ELFMAG, SELFMAG) == 0)
		return true;

	return false;
}
