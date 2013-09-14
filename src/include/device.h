
typedef void (*init_func)();

#define module_init(x) init_func _##x __attribute__((section(".data.init"))) = x

printf("init start %x end %x %x\n", &_init_start, &_init_end, &end);
	
init_func *p;
for(p = (init_func *)&_init_start; p != (init_func *)&_init_end; p++)
{
	(*p)();

}
typedef void (*init_func)();

void init_func_test_func()
{

	printf("awesome\n");
}

init_func init_func_test __attribute__((section(".data.init"))) = init_func_test_func;

extern uintptr_t _init_start, _init_end, end;


