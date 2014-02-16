#include <stdlib.h>
#include <kernel/memory.h>
#include <kernel/common.h>
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <mm/liballoc.h>
#include <stddef.h>

/*
void *calloc(size_t num, size_t size)
{
	return kcalloc(num, size);
}

void free(void *ptr)
{
	kfree(ptr);
}*/

long int strtol(const char* str, char **end, int base)
{
	(void)base;
	long int ret =0;
	int i, sign;
	for(i = 0; isspace(str[i]); i++)
			;
	sign = (str[i] == '-' ? -1 : 1);
	if(str[i] == '-' || str[i] == '+')
		i++;
	
	for(ret = 0; isdigit(str[i]); i++)
	{
		ret = 10 * ret + (str[i] - '0');
	}
	if(end != NULL)
	 *end = (char *)&str[i];
	return ret * sign;
}

