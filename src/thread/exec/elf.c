#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/fs/vfs.h>
#include <chicken/mm/vm.h>
#include <chicken/mm/regions.h>
#include <chicken/thread.h>
#include <chicken/thread/exec.h>

#define ELF_ET_DYN_BASE (PHYS_BASE / 3*2)

int load_elf_internal(executable_t *exe, bool is_interpreter);
static int elf_map_regions(executable_t *exe, Elf32_Ehdr *header, bool is_interpreter UNUSED)
{
	Elf32_Phdr *phdr = kcalloc(sizeof(*phdr), header->e_phnum);

	int ret = exe->inode->read(exe->inode, (void *)phdr, sizeof(*phdr)*header->e_phnum, header->e_phoff);
	if (ret != sizeof(*phdr)*header->e_phnum)
		PANIC("File reading failed while trying to map regions\n");

	uintptr_t adjust = 0;
	if (header->e_type == ET_DYN) {
		if (is_interpreter)
			exe->at_base = adjust = 0xb7f70000;//ELF_ET_DYN_BASE;
			//adjust = 0xb00000;
		else 
			//adjust = 0x8084000;
			adjust = 0x40000;

		exe->at_entry = adjust + header->e_entry;
		exe->at_phdr =  adjust + header->e_phoff;
	} else {

		exe->at_entry = header->e_entry;
		exe->at_phdr =  (exe->at_entry&PAGE_MASK) + header->e_phoff;
	}
	//printf("PHDR %x %x %x\n", adjust, exe->at_entry, exe->at_phdr);
	char *interpreter = NULL;
	for (uint16_t i = 0; i < header->e_phnum; i++, phdr++) {
		if (phdr->p_type == PT_INTERP) {
			size_t len = (size_t)phdr->p_filesz; // includes null terminator
			interpreter = kcalloc(len, 1);
			size_t ret = exe->inode->read(exe->inode, (uint8_t *)interpreter, len, phdr->p_offset);
			if (ret != len)
				PANIC("Error reading interperter name");
			continue;
		}

		if (phdr->p_type != PT_LOAD)
			continue;

		//elf_print_programs(phdr);

		thread_t *cur = thread_current();

;
		uintptr_t vaddr = adjust + phdr->p_vaddr;


		uint32_t mem_length = (vaddr & ~PAGE_MASK) + phdr->p_memsz;
		uint32_t file_length = (phdr->p_offset & ~PAGE_MASK) + phdr->p_filesz;

		// TODO: rename this memregion_map_inode
		memregion_map_file(cur->mm, vaddr, mem_length, PROT_READ|PROT_WRITE|PROT_EXEC,
				MAP_FILE, exe->dentry, phdr->p_offset & PAGE_MASK, file_length);

		//printf("Vaddr %x Filesize %x Memsize %x Offset %x\n", vaddr,
		//	phdr->p_filesz, phdr->p_memsz, phdr->p_offset);
	}

	if (interpreter) {
		executable_t *interp = identify_executable(interpreter, NULL);
		kfree(interpreter);
		if (interp->type != EXE_ELF)
			return -1;
		int ret = load_elf_internal(interp, true);
		if (ret == 0) {
			//printf("ENTRY base %x %x\n", interp->at_base, interp->entry);
			exe->at_base = interp->at_base;;
			exe->ip = interp->ip; // XXX: Hmmmmm
			//printf("EEE %x %x\n", exe->entry, exe->ip);
		}
		return ret;
	} else {
		exe->ip = exe->at_entry;
	}

	return 0;
}

int load_elf_internal(executable_t *exe, bool is_interpreter)
{
	Elf32_Ehdr *header = kmalloc(sizeof(*header));

	int ret = exe->inode->read(exe->inode, (void *)header, sizeof(*header), 0);
	if (ret != sizeof(*header)) {
		printf("Error reading ELF header\nShould have read %x bytes, actually read %x bytes\n",
				sizeof(*header), ret);
		return -1;
	}

	if (memcmp(header->e_ident, ELFMAG, SELFMAG)) {
		printf("Missing or invalid ELF magic number!\n");
		return -1;
	}

	exe->at_phent = sizeof(Elf32_Phdr);
	exe->at_phnum = header->e_phnum;
	ret = elf_map_regions(exe, header, is_interpreter);
	//printf("PHDR %x\n", exe->at_phdr);


	//printf("AT base: %x %x %x %x %x\n", exe->at_base, exe->at_phdr, exe->at_phent, exe->at_phnum, exe->at_entry);

	//printf("ENTRYYY %x IP %x\n", exe->at_entry, exe->ip);

	kfree(header);

	return ret;
}


int load_elf(executable_t *exe)
{
	return load_elf_internal(exe, false);
}