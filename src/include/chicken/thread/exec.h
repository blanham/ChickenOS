#ifndef C_OS_CHICKEN_THREAD_EXEC_H
#define C_OS_CHICKEN_THREAD_EXEC_H
#include <stdint.h>
#include <stdbool.h>
#include <elf.h>

#define MAX_ARGS 256
#define MAX_ENVS 256
#define AT_MAX 37

enum exe_type {
	EXE_INVALID,
	EXE_ELF,
	EXE_SCRIPT
};

uintptr_t stack_prepare(char *path, char *const argv[], char *const envp[]);
enum exe_type exec_type(const char *path);
void duplicate_table(char * dest[], char *const source[]);

void arch_thread_set_ip_and_sp(uintptr_t eip, uintptr_t useresp);
// Elf stuff
bool elf_check_magic(void *magic);
void elf_print_sections(Elf32_Shdr  *sections);
void elf_print_programs(Elf32_Phdr *program);
int load_elf(const char *path, uintptr_t *eip);

#define ELF_MIN_ALIGN PAGE_SIZE
#define ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(ELF_MIN_ALIGN-1))
#define ELF_PAGEOFFSET(_v) ((_v) & (ELF_MIN_ALIGN-1))
#define ELF_PAGEALIGN(_v) (((_v) + ELF_MIN_ALIGN - 1) & ~(ELF_MIN_ALIGN - 1))



#endif
