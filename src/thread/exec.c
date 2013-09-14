#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <mm/liballoc.h>
#include <mm/paging.h>
#include <thread.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>
enum exe_type {EXE_INVALID, EXE_ELF, EXE_SCRIPT};
static uintptr_t stack_prepare(char *path, char *const argv[], int *argvnew, int *argcnew);

enum exe_type exec_type(const char *path)
{
	int fd;
	void *magic;
	enum exe_type ret = EXE_INVALID;
	
	magic = kcalloc(512,1);
	
	fd = sys_open(path, 0);
	
	sys_read(fd, magic, 512);
	
	if(memcmp(magic, ELF_MAGIC, 4) == 0)
		ret = EXE_ELF;
	else if(memcmp(magic, "#!",2) == 0)
		ret = EXE_SCRIPT;
		
	sys_close(fd);
	
	kfree(magic);	
	
	return ret;
}
int sys_execve(const char *path, char *const argv[], char *const envp[]) 
{
	char *name;
	registers_t *regs;
	thread_t *cur = thread_current();
	int newargv, newargc;

	if(path == NULL || argv == NULL)
		return -1;

	
	if(exec_type(path) == EXE_ELF)
	{	
		regs = (void *)((uintptr_t)cur + 4096 - sizeof(*regs));
		//dump_regs(regs);

		//FIXME: this is the wrong place to do this, usually done in init
		{
		sys_open("/dev/tty0", 0);
		sys_open("/dev/tty0", 0);
		sys_open("/dev/tty0", 0);
		}

		//here we need to get rid of old pagetable mappings we are not using
		//I assume we can just use the kernel pagedirectory?
		
		if(load_elf(path, &regs->eip) != 0)
			goto failure;

		if(envp != NULL)
			printf("passing environment not yet supported!\n");

		//backtrack from end of string to get name
		name = (char *)path + strlen(path);
		while((*(--name - 1) != '/') && (name != path));
		
		//FIXME: possible memory leak, but we can't realloc() the original threads ->name	
		cur->name = kcalloc(strlen(name) + 1,1);
		strncpy(cur->name, name, strlen(name) + 1);
	
		//TODO: perhaps just pass these in? or pass in regs	
		regs->useresp = stack_prepare((char *)name, argv, &newargv, &newargc);
		regs->esi = newargc;
		regs->ecx = (uint32_t)newargv;
		
		//FIXME: probably not the best address for break
		cur->brk = (void *)0x80000000;
		pagedir_insert_pagen(cur->pd, (uintptr_t)pallocn(1000), (uintptr_t)cur->brk, 0x7, 1000);
	
	}
	//if we return, then something is wrong	
failure:
	return -1;
}

static void push_arg(char *arg, char **sp)
{
		int len = strlen(arg) + 1;
		*sp -= len;
		strncpy(*sp, arg, len);
		//printf("stack %x string %s *sp %s len %i\n", sp, arg, *sp, len);
}
//TODO: we need a function that deallocates everything on exit()
static uintptr_t stack_prepare(char *path, char *const argv[], int *argvnew, int *argcnew)
{
	int argc = 1;
	char **table;
	char *stack = (void *)PHYS_BASE;//FIXME: Is this right?, or are we stepping on ourselves here?
	char **argvp = (char **)argv;
	
	while(*argvp++ != NULL)	
		argc++;

	table = kcalloc(argc + 1, sizeof(uint32_t *));

	for(int i = argc-2; i >= 0; i--)
	{	
		push_arg(argv[i], &stack);
		table[i+1] = stack;
	}
	
	//push path
	push_arg(path, &stack);
	//first entry of table is path
	table[0] = stack;
	//printf("stack %x\n", stack);

	//keep stack aligned
	stack -= (uintptr_t)stack % 4;
	
	stack -= 4;
	*(uint32_t *)stack = 0;

	stack = stack - (argc * 4);
	kmemcpy(stack, table, argc * 4);
	
	stack -= 4;
	*(uint32_t *)stack = (uint32_t)stack + 4;

//	hex_dump((void *)(PHYS_BASE - 16*3), 3);
	*argvnew = (int)stack;
	*argcnew = argc;
//	printf(" stack %x argvnew %x argcnew %i\n", stack, *argvnew, *argcnew);

	kfree(table);
	return (uintptr_t)stack;
}

