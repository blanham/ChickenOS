#include <chicken/memory.h>

/*
size_t kstrlen(uint8_t *str)
{
	uint8_t *tmp = str;
	while(*tmp != '\0')
		tmp++;
	return tmp - str;
}

uint8_t *kmemcpy(void *dest, const void *src, size_t count)
{
	unsigned char *d = dest;
	const unsigned char *s = src;
	for (; count; count--) *d++ = *s++;
	return dest;
}
*/

uint16_t *kmemcpyw(uint16_t *dest, uint16_t *src, size_t count)
{
	uint16_t *d = dest;
	const uint16_t *s = src;
	for (; count; count--) *d++ = *s++;
	return dest;
}
/*
uint8_t *kmemset(void *dest, uint8_t val, size_t count)
{
	uint8_t *tmp = dest;
	while(count--)
		*tmp++ = val;

	return dest;
}*/

uint16_t *kmemsetw(uint16_t *dest, uint16_t val, size_t count)
{
	uint16_t *tmp = dest;
	while(count--)
		*dest++ = val;

	return tmp;
}
/*
uint32_t *kmemsetl(uint32_t *dest, uint32_t val, size_t count)
{
	uint32_t *tmp = dest;
	while(count--)
		*dest++ = val;

	return tmp;
}*/