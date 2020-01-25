#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/thread.h>
#include <chicken/thread/exec.h>

int load_executable(executable_t *exe, const char *_path,
					char *const _argv[], char *const _envp[])
{
	int ret = -ENOEXEC;
	enum intr_status irq = interrupt_disable();

	// Duplicate all parameters in kernel space as we are about to
	// free the old address space
	char **argv = kcalloc(sizeof(char *), MAX_ARGS);
	char **envp = kcalloc(sizeof(char *), MAX_ARGS);
	char *path = strdup(_path);

	// FIXME: I think the underlying code for this could use some work
	duplicate_table(argv, _argv);
	if(_envp != NULL)
		duplicate_table(envp, _envp);

	// NOTE: After this point all pointers passed in will be invalid, as
	//       the old user stack is cleared
	thread_t *cur = thread_current();
	// TODO: there's probably other things that should be taken care of
	// 		 CLOXEC files for sure, and co-threads in the same process
	mm_init(cur->mm);

	printf("STARTING\n");


retry:
	switch (exe->type) {
		case EXE_ELF:
			if ((ret = load_elf(exe)) != 0) {
				serial_printf("Failed loading file: %i\n", ret);
				goto failure;
			}

			if((ret = stack_prepare(exe, argv[0], argv, envp)) != 0) {
				serial_printf("Failed preparing stack: %i\n", ret);
				goto failure;
			}

			// XXX: rename this and pass it exe
			arch_thread_set_ip_and_usersp(exe->ip, exe->sp);

			serial_printf("execve starting at %x with stack %x\n", exe->ip, exe->sp);
			break;
		case EXE_SCRIPT:
			// TODO: Implement shebang support
			// exe = identify_executable()
			// XXX: Argument passing will be a pain
			PANIC("Shebangs not supported yet!");
			goto retry;
		//case EXE_MODULE:
		default:
			PANIC("INVALID EXE TYPE!");
	}
	printf("Loaded %s\n", path);

	// would it be better to realloc() then strcpyn() here?
	// FIXME: This is the path and not the name, right?
	kfree(cur->name);
	cur->name = strdup(argv[0]);

failure:
	//NOTE: If we end up here this process is fucked as there's nothing to
	//      return to. This should segfault, which I'm fine with
	//      We really shouldn't get here anyway, since all checks that
	//      would result in failure should be covered by exec_type,
	//      including file existence, and ELF header, though if someone
	//      somehow deleted the file between then and here it would fail // XXX: This is accounted for!
	if (ret < 0) {
		// Reduce inode ref count here
		//vfs_close(exe->file);
	}
	kfree(exe);
	kfree(path);
	kfree(argv);
	kfree(envp);
	interrupt_set(irq);
	return ret;
}

int sys_execve(const char *_path, char *const _argv[], char *const _envp[])
{
	// TODO: There should be more verification of pointers here
	if (_path == NULL || _argv == NULL) {
		return -EFAULT;
	}

	executable_t *exe = identify_executable(_path, _argv);
	if (exe->type == EXE_INVALID) {
		return exe->err;
	}

	return load_executable(exe, _path, _argv, _envp);
}