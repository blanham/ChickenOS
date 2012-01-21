#ifndef _ASSERT_H
#define _ASSERT_H
#include <stdio.h>
static inline void assert(int i)
{
	if(i)
		printf("assertion failed!\n");
}
#endif
