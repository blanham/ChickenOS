#include <common.h>
#include <stdio.h>
//#include <string.h>
//#include <memory.h>
//#include <errno.h>
#include <chicken/thread.h>
#include <chicken/thread/exec.h>
//#include <mm/liballoc.h>
//#include <mm/vm.h>
//#include <kernel/interrupt.h>
//#include <fs/vfs.h>
//#include <elf.h>


static void push_arg(char *arg, uint8_t **sp);
static int build_table(char **table, char * const source[], uint8_t **stack);
uintptr_t stack_prepare(char *path, char *const argv[], char *const envp[])
{
	uint8_t *stack = (uint8_t *)PHYS_BASE;
	char **arg_table, **envp_table;
	int argc = 0, envc = 0;

	//alocate these so we don't blow the stack
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

	//TODO: auxv setup should be here
	/*uint32_t *auxv = (void *)regs->useresp + 4;
	kmemset((void *)regs->useresp - 24, 0xff, 48);
//	build_auxv(&auxv);
	regs->useresp = (uintptr_t)auxv;/// */

	return (uintptr_t)stack;
}

//FIXME This should not be using sys_* for file ops
//		Or maybe not? AT_EXECFD is a thing
enum exe_type exec_type(const char *path)
{
	int fd;
	void *magic;
	enum exe_type ret = EXE_INVALID;

	magic = kcalloc(512,1);

	if((fd = sys_open(path, 0, 0)) < 0) {
		printf("exec_type: failed to open file: %s with error: %i\n", path, fd);
		return ret;
	}
	//FIXME: check if file is executable or not
	//       and error if so,probably with fstat
	sys_read(fd, magic, 512);

	if(elf_check_magic(magic))
		ret = EXE_ELF;
	else if(memcmp(magic, "#!",2) == 0)
		ret = EXE_SCRIPT;

	sys_close(fd);

	kfree(magic);

	return ret;
}

void duplicate_table(char * dest[], char *const source[])
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

void build_auxv(uint32_t **auxv)
{
	Elf32_auxv_t *entry = (void *)*auxv;
	#define AT_COUNT 38
	entry->a_type = AT_NULL;
	entry--;
	*auxv = (void *)entry;
}

