#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <thread.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>

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



} elf_section;
/*void stack_prepare(uint32_t *stack, char *const argv[], char *name)
{





}*/

int sys_execv(const char *path, char *const argv[])
{
	int fd;
	elf_header_t *header;
	void *test;
	thread_t *cur;
	registers_t *regs;
//	thread_t *tmp = palloc();
	header = palloc();
	test = palloc();


	cur = thread_current();

	fd = sys_open(path, 0);
//	kmemcpy(tmp, cur, 4096);
	
	printf("Path %s argv[0] %s\n", path, argv[0]);
	
	sys_read(fd, header, 4096);
	sys_read(fd, test, 4096);
	
	pagedir_t pd = cur->pd;

	pagedir_insert_page(pd, (uintptr_t)test, header->entry-0x40, 0x7);

	cur->name = strdup(path);


	regs = (void *)((uintptr_t)cur + 4096 - sizeof(*regs));

	regs->eip = header->entry;
	regs->useresp = PHYS_BASE;

//	printf("phdrpos %X shdrpos %X\n",header->phdrpos, header->shdrpos);
//	printf("phdrent %X phdrcnt %i shdrent %X shdrcnt %i\n",	
//		header->phdrent, header->phdrcnt, header->shdrent, header->shdrcnt);
	
	pagedir_install(pd);	
	
	return -1;

}
