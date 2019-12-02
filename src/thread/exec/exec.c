#include <common.h>
#include <errno.h>
#include <stdio.h>
//#include <string.h>
//#include <memory.h>
#include <chicken/thread.h>
#include <chicken/thread/exec.h>
//#include <mm/liballoc.h>
//#include <mm/vm.h>
//#include <kernel/interrupt.h>
//#include <fs/vfs.h>
//#include <elf.h>

int load_executable(enum exe_type type UNUSED, const char *_path,
					char *const _argv[], char *const _envp[])
{
	thread_t *cur = thread_current();
	int ret = -ENOEXEC;

	char **argv = kcalloc(sizeof(char *), MAX_ARGS);
	char **envp = kcalloc(sizeof(char *), MAX_ARGS);

	//Copy the path into kernel space
	char *path = strdup(_path);

	//duplicate the arg and env tables
	duplicate_table(argv, _argv);
	if(_envp != NULL)
		duplicate_table(envp, _envp);

	//NOTE: After this point all pointers passed in will be invalid, as
	//      the old user stack is cleared
	//mm_clear(cur->mm);
	mm_init(cur->mm);

	uintptr_t ip;
	if((ret = load_elf(path, &ip)) != 0)
	{
		serial_printf("Failed loading file: %i\n", ret);
		//NOTE: If this happens this process is fucked as there's nothing to
		//      return to. This should segfault, which I'm fine with
		//      We really shouldn't get here anyway, since all checks that
		//      would result in failure should be covered by exec_type,
		//      including file existence, and ELF header, though if someone
		//      somehow deleted the file between then and here it would fail
		goto failure;
	}

	kfree(cur->name);
	cur->name = strdup(argv[0]);

	uintptr_t usersp = stack_prepare(cur->name, argv, envp);

	arch_thread_set_ip_and_usersp(ip, usersp);

	serial_printf("execve starting at %x with stack %x\n", ip, usersp);
	//FIXME: probably not the best address for break
	//should be right above the code segment of the
	//executable
	cur->mm->brk = (void *)HEAP_BASE;

failure:
	kfree(path);
	kfree(argv);
	kfree(envp);
	return ret;
}

int sys_execve(const char *_path, char *const _argv[], char *const _envp[])
{
	enum exe_type type = EXE_INVALID;
	int ret = -ENOEXEC;

	//hex_dump((void*)PHYS_BASE-128, 8);

	//printf("EXECVE: PATH %p:%x, %x %x\n", &_path, _path,  _argv, _envp);

	//FIXME: verify pointers here
	//verify_pointer(_path)
	//verify_pointer(_argv)
	//verify_pointer(_envp)

	if (_path == NULL || _argv == NULL) {
		ret = -EFAULT;
		serial_printf("path or argv null\n");
		goto failure;
	}

	type = exec_type(_path);
	serial_printf("TYPE %i\n", type);

	switch(type)
	{
		case EXE_SCRIPT:
			printf("Hashbangs not yet supported\n");
			ret = -ENOEXEC;
			break;
		case EXE_ELF:
			ret = load_executable(type, _path, _argv, _envp);
			break;
		case EXE_INVALID:
		default:
			ret = -ENOEXEC;
	}

failure:
	return ret;
}

