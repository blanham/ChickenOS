#include <common.h>
#include <errno.h>
#include <stdio.h>
#include <chicken/thread.h>
#include <chicken/thread/exec.h>

static void push_arg(char *arg, uint8_t **sp);
static int build_table(char **table, char * const source[], uint8_t **stack);
int stack_prepare(executable_t *exe, char *path, char *const argv[], char *const envp[])
{
	uint8_t *stack = (uint8_t *)PHYS_BASE;

	// Allocate these so we don't blow the stack
	char **arg_table = kcalloc(MAX_ARGS, sizeof(uint32_t *));
	char **envp_table = kcalloc(MAX_ENVS, sizeof(uint32_t *));

	// First thing on stack is an end marker (0)
	*(uint32_t *)stack-- = 0;

	// XXX: This is necessary for AT_EXECFN
	// 		So we need to push this value it
	push_arg(path, &stack);

	int envc = build_table(envp_table, envp, &stack);
	if (envc < 0) {
		//XXX: More error checking would be nice
		return -E2BIG;
	}
	int argc = build_table(arg_table, argv, &stack);
	if (envc < 0) {
		return -E2BIG;
	}

	// Align stackpointer to a 16 byte boundary
	stack -= (uintptr_t)stack & 0xF;

	//stack -= AT_MAX * sizeof(Elf32_auxv_t);

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

	//hex_dump((void *)(PHYS_BASE - 16*8), 8);

	kfree(arg_table);
	kfree(envp_table);

	exe->sp = (uintptr_t)stack;

	return 0;
}

#define BUF_SIZE 512
executable_t *identify_executable(const char *path, char *const _argv[])
{

	// FIXME: remove this:
	(void)_argv;
	executable_t *ret = kcalloc(sizeof(*ret), 1);
	// FIXME: Figure out correct flags to pass to open()
	// TODO: mark this inode so that it can't be written to
	struct file * file = vfs_open((char *)path, 0, 0);
	if(file == NULL) {
		printf("exec_type: failed to open file: %s with error: (failure)\n", path);

		// XXX: This isn't right, once I integrate the new fs code
		//		we will have a vfs_open that will handle all of the I/O errors
		ret->err = -1;
		PANIC("EXE loading failed, and the error handling isn't finished yet");
		return ret;
	}
	uint8_t *magic = kcalloc(BUF_SIZE, 1);
	vfs_read(file, magic, BUF_SIZE);

	if(memcmp(magic, "#!",2) == 0) {
		// TODO: Implement shabangs
		PANIC("We don't handle shabangs yet");
	}

	if(elf_check_magic(magic)) {
		ret->file = file;
		ret->type = EXE_ELF;
	}

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

