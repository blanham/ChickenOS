#include <stdio.h>
#include "bitmap.h"

uint32_t array[1024];
bitmap_t bitmap = array;

int main(int argc, char **argv)
{
//	bitmap_set(bitmap, 1024);
	int i;
	for(i = 0; i < 1024; i++)
		bitmap_set(bitmap, i);
//	int ret= bitmap_check(bitmap,1024);
	
//	printf("ret %i\n",ret);
	bitmap_clear(bitmap, 333);
	printf("free %i\n",bitmap_find_first(bitmap));

}
