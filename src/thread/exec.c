#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <mm/liballoc.h>
#include <mm/paging.h>
#include <thread.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>

#define MAX_ARGS 256
#define MAX_ENVS 256

enum exe_type {EXE_INVALID, EXE_ELF, EXE_SCRIPT};

static uintptr_t stack_prepare(char *path, char *const argv[], char *const envp[]);
static enum exe_type exec_type(const char *path);




static void duplicate_table(char * dest[], char *const source[]);
int sys_execve(const char *_path, char *const _argv[], char *const _envp[]) 
{
	registers_t *regs;
	thread_t *cur = thread_current();
	uint32_t *user_stack;
	char **argv;
	char **envp;
	char *path;
	
	argv = kcalloc(sizeof(char *), MAX_ARGS);
	envp = kcalloc(sizeof(char *), MAX_ARGS);

	
	if(_path == NULL || _argv == NULL)
		goto failure;
	
	path = strdup(_path);
	
	duplicate_table(argv, _argv);
	if(_envp != NULL)
	duplicate_table(envp, _envp);

	if(exec_type(path) == EXE_ELF)
	{
		//FIXME: This is shitty, should either get regs from
		//the threads cur->regs, or have them passed in
		regs = (void *)((uintptr_t)cur + 4096 - sizeof(*regs));
		
		user_stack = palloc();
		kmemset(user_stack,0, 4096);

		//FIXME: some things may carry over	
		cur->pd = pagedir_new();
		
		pagedir_insert_page(cur->pd, (uintptr_t)user_stack, 
			(uintptr_t)PHYS_BASE - 0x1000, 0x7);

		pagedir_install(cur->pd);
		
	
		if(load_elf(path, &regs->eip) != 0)
			goto failure;

		//backtrack from end of string to get name
		//FIXED: Might not have to do this, just get name from argv[0]
		//name = (char *)path + strlen(path);
		//while((*(--name - 1) != '/') && (name != path));
		
		kfree(cur->name);
		cur->name = strdup(argv[0]);
	
		regs->useresp = stack_prepare(cur->name, argv, envp);
		
		//FIXME: probably not the best address for break
		//should be right above the code segment of the
		//executable
		cur->brk = (void *)0x84000000;

		pagedir_insert_pagen(cur->pd, (uintptr_t)pallocn(1000), (uintptr_t)cur->brk, 0x7, 1000);
	}

	//if we return, then something is wrong	
failure:
	return -1;
}

static void push_arg(char *arg, uint8_t **sp);
static int build_table(char **table, char * const source[], uint8_t **stack);
static uintptr_t stack_prepare(char *path, char *const argv[], char *const envp[])
{
	uint8_t *stack = (uint8_t *)PHYS_BASE;
	char **arg_table;//[MAX_ARGS];
	char **envp_table;//[MAX_ENVS];
	int argc = 0, envc = 0;	

	//alocate these so we doun't blow the stack	
	arg_table = kcalloc(MAX_ARGS, sizeof(uint32_t *));
	envp_table = kcalloc(MAX_ENVS, sizeof(uint32_t *));


	memset(arg_table, 0, sizeof(char *)*MAX_ARGS);
	memset(envp_table, 0, sizeof(char *)*MAX_ENVS);

	//first thing on stack is 0
	*(uint32_t *)stack = 0;
	stack -= 4;

	//then the executable path
	push_arg(path, &stack);

	//the environment, and then arguments
	envc = build_table(envp_table, envp, &stack);
	argc = build_table(arg_table, argv, &stack);
	
	//align stackpointer to 4 byte boundary
	stack -= ( ((uintptr_t)stack %4));
	
	//copy tables into stack
	stack -= 4;
	stack -= envc*4;
	memcpy(stack, envp_table, envc*4);
	
	stack -= 4;
	stack -= argc*4;
	memcpy(stack, arg_table, argc*4);
	
	//put argc at top of stack
	stack -= 4;
	*(uint32_t *)stack = argc;

	//hex_dump((void *)(PHYS_BASE - 16*4), 4);

	kfree(arg_table);
	kfree(envp_table);
	
	return (uintptr_t)stack;	
}

static enum exe_type exec_type(const char *path)
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

static void duplicate_table(char * dest[], char *const source[])
{
	char **p;
	int count =0;
	
	for(p = (char **)source; *p != NULL; p++)
	{	
		dest[count] = strdup(*p);
		count++;
	}
	
	dest[count] = 0;
}  

static void push_arg(char *arg, uint8_t **sp)
{
		int len = strlen(arg) + 1;
		*sp -= len;
		strncpy((char *)*sp, arg, len);
}

static int build_table(char **table, char * const source[], uint8_t **stack)
{
	char **p = (char **)source;
	int arg_count = 0;
	while(*p != NULL)
	{
		push_arg(*p++, stack);
		table[arg_count] = (char *)*stack;
		arg_count++;
	}

	return arg_count;
}

