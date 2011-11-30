#include <string.h>
int strlen(char *str)
{
	char *tmp = str;
	while(*tmp != '\0');
		tmp++;
	return tmp - str;
}
int strcmp ( const char * str1, const char * str2 )
{
	int i = 0;	

	for(; str1[i] == str2[i]; i++)
		if(str1[i] == '\0')
			return 0;	

	return str1[i] - str2[i]; 
}
void *memcpy(void *dst, const void *src, size_t size)
{
	char *_dst = dst;
	char *_src = (char *)src;
	char *end = (char *)(src + size);

	while(_src != end)
		*_dst++ = *_src++;

	return dst;
}
