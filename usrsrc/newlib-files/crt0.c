//#include <reent.h>
extern int main(); //int argc, char **argv, char **environ);

extern char __bss_start, _end; // BSS should be the last think before _end
extern char* sbrk_base;
// XXX: environment

char *___argv[] = {"dash",  0};
char *__env[1] = { 0 };
char **environ = __env;
//extern void __do_global_ctors_aux();
//struct _reent DATA;
//extern void __do_global_ctors();
_start(){
	int argc;
	char **argv;
	asm volatile ( "mov %%esi, %0":"=m"(argc)  );
	asm volatile ( "mov %%ecx, %0":"=m"(argv)  );
  char *i;
//	init();
//	__do_global_ctors_aux();
	int ret;
  // zero BSS
  for(i = &__bss_start; i < &_end; i++){
    *i = 0; 
  } 
//	sbrk_base = sbrk(0);
//	_impure_ptr = &DATA;
//	_REENT_INIT_PTR(_impure_ptr);
//__sinit(_impure_ptr);
  // XXX: get argc and argv
  ret = main(argc, argv, __env);

//while(1);
	exit(ret);
}
