#include <common.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <mm/liballoc.h>
#include <thread.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>
enum exe_type {EXE_INVALID, EXE_ELF};
static uintptr_t stack_prepare(char *path, char *const argv[], int *argvnew, int *argcnew);

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
			printf("%c",isprint(p[i]) == 0 ? '.' : p[i]);
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
	
	if(memcmp(magic, ELF_MAGIC, 4) == 0)
		ret = EXE_ELF;
		
	sys_close(fd);
	
	palloc_free(magic);
	
	return ret;
}
int sys_execve(const char *path, char *const argv[], char *const envp[]) 
{
	char *name;
	static int tmp = 0;
	registers_t *regs;
	thread_t *cur = thread_current();

	//while(1);

	if(path == NULL || argv == NULL)
		return -1;

//	if((uintptr_t)path < 0x30303040)
//		return -1;
	
if(exec_type(path) == EXE_ELF)
	{	
	//	uintptr_t eip;
		regs = (void *)((uintptr_t)cur + 4096 - sizeof(*regs));
	//	dump_regs(regs);

		//FIXME: this is the wrong place to do this
		if(tmp == 0)
		{
			sys_open("/dev/tty0", 0);
			sys_open("/dev/tty0", 0);
			sys_open("/dev/tty0", 0);
		}else{
			sys_open("/dev/tty1", 0);
			sys_open("/dev/tty1", 0);
			sys_open("/dev/tty1", 0);

		}
		tmp++;

		if(load_elf(path, &regs->eip) != 0)
			goto failure;

	//	printf("loaded elf\n");

		if(envp != NULL)
			printf("passing environment not yet supported!\n");

	//	printf("argv %s\n", argv[0]);	
	//	regs->eip = eip;

		int newargv;
		int newargc;
				//backtrack from end of string to get name
		name = (char *)path + strlen(path);
		while((*(--name - 1) != '/') && (name != path));

		cur->name = krealloc(cur->name, strlen(name) + 1);
		regs->useresp = stack_prepare((char *)name, argv, &newargv, &newargc);
		regs->esi = newargc;
		regs->ecx = (uint32_t)newargv;

	//	strcpy(cur->name, name);
	//	printf("here?\n");	
		//FIXME: probably not the best address for break
		cur->brk = (void *)0x80000000;
		pagedir_insert_pagen(cur->pd, (uintptr_t)pallocn(1000), (uintptr_t)cur->brk, 0x7, 1000);
		//thread_yield();
	//	printf("done\n");
	}
	//if we return, then something is wrong	
failure:
	return -1;
}
static void push_arg(char *arg, char **sp)
{
		int len;
		len = strlen(arg) + 1;
		*sp -= len;
		strcpy(*sp, arg);
		printf("stack %x string %s *sp %s len %i\n", sp, arg, sp, len);
}
static uintptr_t stack_prepare(char *path, char *const argv[], int *argvnew, int *argcnew)
{
	int argc = 1;
	char **table;
	//uint32_t *stackw;
	char *stack = (void *)PHYS_BASE;
	char **argvp = (char **)argv;
	
	while(*argvp++ != NULL)	
		argc++;

	table = kcalloc(argc + 1, sizeof(uint32_t *));


	(void)path;
	
	for(int i = argc-2; i >= 0; i--)
	{	
		push_arg(argv[i], &stack);
		table[i+1] = stack;
	}
	
	//push path
	push_arg(path, &stack);
	//first entry of table is path
	table[0] = stack;
	printf("stack %x\n", stack);

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

