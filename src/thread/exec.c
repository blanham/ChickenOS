#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <mm/liballoc.h>
#include <mm/paging.h>
#include <thread.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>
#include <elf.h>
#define MAX_ARGS 256
#define MAX_ENVS 256

enum exe_type {EXE_INVALID, EXE_ELF, EXE_SCRIPT};

static uintptr_t stack_prepare(char *path, char *const argv[], char *const envp[]);

static enum exe_type exec_type(const char *path);


char *envpr[] = {"S"," ", "","","","","","","",""};

static void duplicate_table(char * dest[], char *const source[]);
void build_auxv(uint32_t **auxv)
{
	Elf32_auxv_t *entry = (void *)*auxv;
	#define AT_COUNT 38
	entry->a_type = AT_NULL;
	entry--;
	*auxv = (void *)entry;
}
//FIXME: Doesn't pass an auxv 
int sys_execve(const char *_path, char *const _argv[], char *const _envp[]) 
{
	registers_t *regs;
	thread_t *cur = thread_current();
	uint32_t *user_stack;
	char **argv = NULL, **envp = NULL;
	char *path;
	enum exe_type type;

	if(_path == NULL || _argv == NULL)
		goto failure;

	type = exec_type(_path);

	switch(type)
	{
		case EXE_SCRIPT:
			printf("Hashbangs not yet supported\n");
			return -ENOEXEC;
		case EXE_ELF:
			break;
		default:
			return -ENOEXEC;
	}

	{
		argv = kcalloc(sizeof(char *), MAX_ARGS);
		envp = kcalloc(sizeof(char *), MAX_ARGS);

		path = strdup(_path);
		duplicate_table(argv, _argv);

		if(_envp != NULL)
			duplicate_table(envp, _envp);

		regs = (void *)cur + STACK_SIZE - sizeof(*regs);
		
		//FIXME: We already have a user stack?
		user_stack = palloc();
		kmemset(user_stack,0, 4096);

		//FIXME: some things may carry over	
		cur->pd = pagedir_new();
		
		pagedir_insert_page(cur->pd, (uintptr_t)user_stack, 
			(uintptr_t)PHYS_BASE - 0x1000, 0x7);

		pagedir_install(cur->pd);
	
		if(load_elf(path, &regs->eip) != 0)
			goto failure;

		kfree(cur->name);
		cur->name = strdup(argv[0]);
	
		regs->useresp = stack_prepare(cur->name, argv, envp);
		/*uint32_t *auxv = (void *)regs->useresp + 4;
		kmemset((void *)regs->useresp - 24, 0xff, 48);
	//	build_auxv(&auxv);
		regs->useresp = (uintptr_t)auxv;/// */


		printf("execve starting at %x with stack %x\n", regs->eip, regs->useresp);	
		//FIXME: probably not the best address for break
		//should be right above the code segment of the
		//executable, 
		cur->brk = (void *)0xA400000;

		pagedir_insert_pagen(cur->pd, (uintptr_t)pallocn(1000), (uintptr_t)cur->brk, 0x7, 1000);
	}

	//if we return, then something is wrong	
failure:
	kfree(argv);
	kfree(envp);
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
	
	if((fd = sys_open(path, 0, 0)) < 0)
		return -1;
	
	sys_read(fd, magic, 512);
	
	if(elf_check_magic(magic))
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

