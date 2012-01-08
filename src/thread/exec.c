#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <mm/liballoc.h>
#include <thread.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>
#define ELF_MAGIC "\177ELF"
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
	uint32_t type;
	uint32_t offset;
	uint32_t virtaddr;
	uint32_t physaddr;
	uint32_t filesize;
	uint32_t memsize;
	uint32_t flags;
	uint32_t align;
} __attribute__((packed)) elf_program_header_t;
static uintptr_t stack_prepare(char *path, char *const argv[]);

static void elf_print_sections(elf_section_t *sections)
{
//	uint32_t sh_name;
//	uint32_t sh_type;
//	uint32_t sh_flags;
	printf("type %i addr %X offset %i size %X link %X info %X align %X entsize %x\n",
 		sections->sh_type, sections->sh_addr, sections->sh_offset, sections->sh_size,
		sections->sh_link, sections->sh_info, sections->sh_align, sections->sh_entsize);

}

static void elf_print_programs(elf_program_header_t *program)
{
	printf("type %i offset %X virtaddt %X filesize %X memsize %X align %X\n",
		program->type, program->offset, program->virtaddr,
 		program->filesize, program->memsize, program->align);
}
int isprint(char c)
{
	if(('a' <= c) && (c <= 'z'))
	return 1;
	if(c == '/')
		return 1;
	if(c == '-')
		return 1;
	return 0;

}
void hex_dump(void *ptr)
{
	uint8_t *p = ptr;	
	for(int j =0; j < 4; j++)
	{
		printf("%X ",p);
		for(int i = 0; i < 16; i++)
		{
			printf("%.2X ",p[i]);
		}
		printf("|");
		for(int i = 0; i < 16; i++)
		{
			printf("%c",isprint(p[i]) == 0 ? '.' : p[i]);
		}
		printf("|");
			printf("\n");
		p+=16;
	}
}

/*int load_page(int fd, virtaddr_t )
{
} */
static int load_elf(const char *path)
{
	int fd;
	elf_header_t *header;
	elf_section_t *sections;
	elf_program_header_t *program;
	void *test, *test2;
	thread_t * cur;
	uintptr_t entry;
	cur = thread_current();
	header = kmalloc(sizeof(*header));
	test = palloc();
	test2 = palloc();

	fd = sys_open(path, 0);
	path = strdup("/bin/ls");
	
	sys_read(fd, header, sizeof(*header));
	program = kmalloc(sizeof(*program)*header->phdrcnt);
	sys_lseek(fd, header->phdrpos, SEEK_SET);
	sys_read(fd, program, sizeof(*program)*header->phdrcnt);

	for(int i = 0; i < header->phdrcnt; i++)
	{
		elf_print_programs(program + i);

	}
	sys_lseek(fd, header->shdrpos, SEEK_SET);
	sections = kmalloc(sizeof(*sections)*header->shdrcnt);
	sys_read(fd, sections, sizeof(*sections)*header->shdrcnt);
	for(int i = 0; i < header->shdrcnt -10; i++)
	{
		elf_print_sections(sections + i);

	}
//	if(memcmp(header->magic, ELF_MAGIC, 4))
	//	printf("ELF GOOD\n");
	sys_lseek(fd, 4096, SEEK_SET);
	sys_read(fd, test, 4096);
	sys_read(fd, test2, 4096);
	void *test3 = palloc();
	sys_read(fd, test3, 4096);
	
	sys_close(fd);

	pagedir_t pd = cur->pd;

	pagedir_insert_page(pd, (uintptr_t)test, header->entry & ~0xFFF, 0x7);
	pagedir_insert_page(pd, (uintptr_t)test2, (header->entry & ~0xFFF) + 0x1000, 0x7);
	pagedir_insert_page(pd, (uintptr_t)test3, (header->entry & ~0xFFF) + 0x2000, 0x7);

	pagedir_install(pd);
	
//	PRIntf("phdrpos %X shdrpos %X\n",header->phdrpos, header->shdrpos);
//	printf("phdrent %X phdrcnt %i shdrent %X shdrcnt %i\n",	
//		header->phdrent, header->phdrcnt, header->shdrent, header->shdrcnt);
	
	entry = header->entry;
	kfree(header);
	kfree(sections);
	return entry;
}
enum exe_type {EXE_INVALID, EXE_ELF};
enum exe_type exec_type(const char *path UNUSED)
{
	int fd;
	void *magic;
	enum exe_type ret = EXE_INVALID;
	magic = palloc();
	
	fd = sys_open(path, 0);
	sys_read(fd, magic, 4096);
	if(memcmp(magic, ELF_MAGIC, 4))
		ret = EXE_ELF;
		
	sys_close(fd);
	palloc_free(magic);
	return ret;
}
int sys_execv(const char *path, char *const argv[])
{
	char *name;
	registers_t *regs;
	thread_t *cur = thread_current();
	

	//backtrack from end of string to get name
	name = (char *)path + strlen(path);
	while((*(--name - 1) != '/') && (name != path));
	if(exec_type(path) == EXE_ELF)
	{	
		uintptr_t eip;
		regs = (void *)((uintptr_t)cur + 4096 - sizeof(*regs));
	
		if((int)(eip = (uintptr_t)load_elf(path)) == -1)
			goto failure;
	
		regs->eip = eip;
		regs->useresp = stack_prepare((char *)path, argv);

		
		cur->name = krealloc(cur->name, strlen(name) + 1);
		strcpy(cur->name, name);
	}
	//if we return, then something is wrong		
failure:
	return -1;
}
static void *push_arg(char *arg, void *sp)
{

		int len;
		len = strlen(arg) + 1;
		sp -= len;
		strcpy((char *)sp, arg);	
		return sp;
}
static uintptr_t stack_prepare(char *path, char *const argv[])
{
	int argc = 1;
	char **table;
	uint32_t *stackw;
	char *stackc = (void *)PHYS_BASE;
	char **argvp = (char **)argv;
	
	while(*argvp++ != NULL)	
		argc++;

	table = kcalloc((argc + 1)*sizeof(uint32_t *), 1);
	
	for(int i = argc-2; i >= 0; i--)
	{	
		stackc = push_arg(argv[i], stackc);
		table[i+1] = stackc;
	}	
	//push path
	stackc = push_arg(path, stackc);
	//first entry of table is path
	table[0] = stackc;

	//keep stack aligned
	if(((uintptr_t)stackc % 4) != 0)
		for(uint32_t i = 0; i < ((uintptr_t)stackc % 4); i++)
			*--stackc = 0;

	stackw = (uint32_t *)stackc;

	//FIXME: should be able to use the NULL on the end of the
	//supplied argv[]
	//push null pointer (argv[argc] == NULL)
	*--stackw = NULL;

	//push table (argv) onto stack
	for(int i = argc; i >= 0; i--)
		*stackw-- = (uint32_t)table[i];
	
	//gcc will not let this be one line
	*stackw = (uint32_t)stackw + 4;
	stackw--;

	*stackw-- = argc;
	//NULL function return value (entry point should not return)
	*stackw = NULL;
	
//	hex_dump((void *)(PHYS_BASE - 64));

	kfree(table);
	return (uintptr_t)stackw;
}

