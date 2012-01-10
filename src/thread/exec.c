#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <mm/liballoc.h>
#include <thread.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>
enum exe_type {EXE_INVALID, EXE_ELF};
static uintptr_t stack_prepare(char *path, char *const argv[]);
static inline int _isprint(char c)
{
	if(('a' <= c) && (c <= 'z'))
	return 1;
	if(c == '/')
		return 1;
	if(c == '-')
		return 1;
	return 0;

}
void hex_dump(void *ptr, int n)
{
	uint8_t *p = ptr;	
	for(int j =0; j < n; j++)
	{
		printf("%X ",p);
		for(int i = 0; i < 16; i++)
		{
			printf("%.2X ",p[i]);
		}
		printf("|");
		for(int i = 0; i < 16; i++)
		{
			printf("%c",_isprint(p[i]) == 0 ? '.' : p[i]);
		}
		printf("|");
		printf("\n");
		p+=16;
	}
}
enum exe_type exec_type(const char *path)
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
int sys_execve(const char *path, char *const argv[], char *const envp[]) 
{
	char *name;
	registers_t *regs;
	thread_t *cur = thread_current();
	printf("path %s argv0 %s\n",path, argv[0]);	
	if(exec_type(path) == EXE_ELF)
	{	
	//	uintptr_t eip;
		regs = (void *)((uintptr_t)cur + 4096 - sizeof(*regs));
		
		sys_open("/dev/tty", 0);
		sys_open("/dev/tty", 0);
		
		if(load_elf(path, &regs->eip) != 0)
			goto failure;
		if(envp != NULL)
			printf("passing environment not yet supported!\n");	
	//	regs->eip = eip;
		regs->useresp = stack_prepare((char *)path, argv);
		
		//backtrack from end of string to get name
		name = (char *)path + strlen(path);
		while((*(--name - 1) != '/') && (name != path));

		cur->name = krealloc(cur->name, strlen(name) + 1);
		strcpy(cur->name, name);
		
		//FIXME: probably not the best address for break
		cur->brk = (void *)0xb0000000;
		pagedir_insert_page(cur->pd, (uintptr_t)palloc(), (uintptr_t)cur->brk, 0x7);
		printf("done\n");
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

