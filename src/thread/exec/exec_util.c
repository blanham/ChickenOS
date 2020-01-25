#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/thread.h>
#include <chicken/thread/exec.h>


// TODO: mark this inode so that it can't be written to
executable_t *identify_executable(const char *path, char *const _argv[] UNUSED)
{
	executable_t *ret = kcalloc(sizeof(*ret), 1);

	dentry_t *lookup = NULL;
	int err_ret = vfs_pathsearch(path, &lookup, NULL);
	if (lookup == NULL) {
		printf("exec_type: failed to open file: %s with error: %i\n", path, -err_ret);

		// XXX: This isn't right, once I integrate the new fs code
		//		we will have a vfs_open that will handle all of the I/O errors
		ret->err = err_ret;
		PANIC("EXE loading failed, and the error handling isn't finished yet");
		return ret;
	}

	// TODO: check permissions here

	uint8_t *magic = kcalloc(4096, 1);
	err_ret = lookup->inode->read(lookup->inode, magic, 4096, 0);
	if (err_ret != PAGE_SIZE) {
		PANIC("EXE loading failed, and the error handling isn't finished yet");
		ret->err = err_ret;
		return ret;
	}

	if (!memcmp(magic, "#!", 2)) {
		ret->inode = lookup->inode;
		ret->type = EXE_SCRIPT;
	}

	if (elf_check_magic(magic)) {
		ret->inode = lookup->inode;
		ret->type = EXE_ELF;
	}

	ret->dentry = lookup;

	kfree(magic);
	return ret;
}

void duplicate_table(char * dest[], char *const source[])
{
	int count = 0;
	for (char **p = (char **)source; *p != NULL; p++)
		dest[count++] = strdup(*p);
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
	int arg_count = 0;
	while (*source) {
		push_arg(*source++, stack);
		table[arg_count] = (char *)*stack;
		arg_count++;
	}
	return arg_count;
}

int random_bytes(uint8_t *dst, size_t count)
{
	// XXX: figure out best/simplest way to generate random bytes soon, proper random support later
	memset(dst, 0x42, count);
	return 0;
}

static uint8_t *build_auxv(executable_t *exe, char *at_execfn, void *stack)
{
	(void)exe;

	stack -= strlen("i686") + 1;
	char *at_platform = stack;
	strcpy(at_platform, "i686");

	// AT_RANDOM points to 16 bytes of randomness
	stack -= 16;
	uint8_t *at_random = stack;
	random_bytes(at_random, 16);

	stack -= (uintptr_t)stack & 0xF;

	Elf32_auxv_t *entry = stack;

	entry--;
	entry->a_type = AT_NULL;
	entry->a_un.a_val = 0;

	entry--;
	entry->a_type = AT_PLATFORM;
	entry->a_un.a_val = (uintptr_t)at_platform;

	entry--;
	entry->a_type = AT_EXECFN;
	entry->a_un.a_val = (uintptr_t)at_execfn;

	//entry--;
	//entry->a_type = AT_HWCAP2;
	//entry->a_un.a_val = 0;

	entry--;
	entry->a_type = AT_RANDOM;
	entry->a_un.a_val = (uintptr_t)at_random;

	entry--;
	entry->a_type = AT_SECURE;
	entry->a_un.a_val = 0;

	// FIXME: pass the proper UID etc here
	entry--;
	entry->a_type = AT_EGID;
	entry->a_un.a_val = 1000;

	entry--;
	entry->a_type = AT_GID;
	entry->a_un.a_val = 1000;

	entry--;
	entry->a_type = AT_EUID;
	entry->a_un.a_val = 1;

	entry--;
	entry->a_type = AT_UID;
	entry->a_un.a_val = 1;

	// We need AT_ENTRY, AT_BASE, AT_PHDR, AT_PHENT, AT_PHNUM ... for dynamic linking
	entry--;
	entry->a_type = AT_ENTRY;
	entry->a_un.a_val = exe->at_entry;

	entry--;
	entry->a_type = AT_FLAGS;
	entry->a_un.a_val = exe->at_flags; // XXX: is this right?

	entry--;
	entry->a_type = AT_BASE;
	entry->a_un.a_val = exe->at_base;

	entry--;
	entry->a_type = AT_PHNUM;
	entry->a_un.a_val = exe->at_phnum;

	entry--;
	entry->a_type = AT_PHENT;
	entry->a_un.a_val = exe->at_phent;

	entry--;
	entry->a_type = AT_PHDR;
	entry->a_un.a_val = exe->at_phdr;

	entry--;
	entry->a_type = AT_CLKTCK;
	entry->a_un.a_val = 100; // XXX: this should be a define somewhere

	entry--;
	entry->a_type = AT_PAGESZ;
	entry->a_un.a_val = PAGE_SIZE;
	
	//entry--;
	//entry->a_type = AT_HWCAP;
	//entry->a_un.a_val = 0;

	//entry--;
	//entry->a_type = AT_SYSINFO; // AT_SYSINFO_EHDR?
	//entry->a_un.a_val = 0x0;
	//entry--;
	//entry->a_type = AT_SYSINFO_EHDR; // AT_SYSINFO_EHDR?
	//entry->a_un.a_val = 0x0;
	// TODO: MINSIGSTKSIZ is used by musl AIO

	return (uint8_t *)entry;
}

int stack_prepare(executable_t *exe, char *path, char *const argv[], char *const envp[])
{
	uint8_t *stack = (uint8_t *)PHYS_BASE;

	// Allocate these so we don't blow the stack
	char **arg_table = kcalloc(MAX_ARGS, sizeof(uintptr_t *));
	char **envp_table = kcalloc(MAX_ENVS, sizeof(uintptr_t *));

	// First thing on stack is an end marker (0)
	*(uintptr_t *)--stack = 0;

	// Then the program name
	push_arg(path, &stack);

	char *at_execfn = (char *)stack;

	int envc = build_table(envp_table, envp, &stack);
	if (envc < 0) {
		//XXX: More error checking would be nice
		return -E2BIG;
	}
	int argc = build_table(arg_table, argv, &stack);
	if (argc < 0) {
		return -E2BIG;
	}

	// Align stackpointer to a 16 byte boundary
	stack -= (uintptr_t)stack & 0xF;
	
	// Populate auxiliary vector
	stack = build_auxv(exe, at_execfn, stack);

	//copy tables into stack
	stack -= 4;
	stack -= envc*4;
	memcpy(stack, envp_table, envc*4);

	stack -= 4;
	stack -= argc*4;
	memcpy(stack, arg_table, argc*4);

	//put argc at top of stack
	stack -= 4;
	*(uintptr_t *)stack = argc;

	//hex_dump((void *)(PHYS_BASE - 16*8), 8);

	kfree(arg_table);
	kfree(envp_table);

	exe->sp = (uintptr_t)stack;

	return 0;
}
