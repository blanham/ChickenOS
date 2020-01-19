#ifndef C_OS_CHICKEN_THREAD_EXEC_H
#define C_OS_CHICKEN_THREAD_EXEC_H
#include <stdint.h>
#include <stdbool.h>
#include <elf.h>
#include <fs/vfs.h>

#define MAX_ARGS 256
#define MAX_ENVS 256
#define AT_MAX 37

enum exe_type {
	EXE_INVALID = 0,
	EXE_ELF,
	EXE_SCRIPT
};

typedef struct {
	enum exe_type type;
	union {
		int err;
		// FIXME: Should be dentry_t
		struct inode *inode;
	};
	uintptr_t ip;
	uintptr_t sp;

	uintptr_t at_entry;
	uintptr_t at_flags;
	uintptr_t at_base;
	uintptr_t at_phnum;
	uintptr_t at_phent;
	uintptr_t at_phdr;
} executable_t;

int stack_prepare(executable_t *exe, char *path, char *const argv[], char *const envp[]);
//executable_t *identify_executable(const char *path);
executable_t *identify_executable(const char *path, char *const _argv[]);
void duplicate_table(char * dest[], char *const source[]);

void arch_thread_set_ip_and_sp(uintptr_t eip, uintptr_t useresp);
// Elf stuff
bool elf_check_magic(void *magic);
void elf_print_sections(Elf32_Shdr  *sections);
void elf_print_programs(Elf32_Phdr *program);
int load_elf(executable_t *exe);

#define ELF_MIN_ALIGN PAGE_SIZE
#define ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(ELF_MIN_ALIGN-1))
#define ELF_PAGEOFFSET(_v) ((_v) & (ELF_MIN_ALIGN-1))
#define ELF_PAGEALIGN(_v) (((_v) + ELF_MIN_ALIGN - 1) & ~(ELF_MIN_ALIGN - 1))



#endif
