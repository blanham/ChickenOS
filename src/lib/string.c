#include <string.h>
int strlen(char *str)
{
	char *tmp = str;
	while(*tmp != '\0');
		tmp++;
	return tmp - str;
}

void *memcpy(void *dst, const void *src, size_t size)
{
	char *_dst = dst;
	char *_src = (char *)src;
	char *end = (char *)(src + size);

	while(_src != end)
		*_dst = *_src;

	return dst;
}
