#include <stdlib.h>
#include <kernel/memory.h>
#include <kernel/common.h>
#include <kernel/interrupt.h>
#include <kernel/types.h>
#include <kernel/vm.h>
#include <mm/liballoc.h>
#include <stddef.h>


void *calloc(size_t num, size_t size)
{

	return kcalloc(num, size);
}
