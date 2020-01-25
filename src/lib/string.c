#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * strcat(char *restrict dst, const char *restrict src)
{
	strcpy(dst + strlen(dst), (char *)src);
	return dst;
}

char * strncat(char *dst, const char * src, size_t len)
{
	char *ret = dst;
	dst += strlen(dst);
	while(((*dst++ = *src++)) && (--len > 0));
	*dst = '\0';
	return ret;
}

int strcmp(const char * str1, const char * str2)
{
	while (*str1 && *str1++ == *str2++);
	return *(unsigned char *)str1 - *(unsigned char *)str2;
}

// This is trickier then you would expect: https://stackoverflow.com/questions/1356741/strcmp-and-signed-unsigned-chars
int strncmp(const char *str1, const char *str2, size_t num)
{
	if (!num--) return 0;
	for (;*str1 && *str2 && num && *str1 == *str2; str1++, str2++, num--);
	return *(unsigned char *)str1 - *(unsigned char *)str2;
}

char * strchr(const char *str, int c)
{
/*	char *_str = (char *)str;

	while(1)
		if(*_str == (char)c)
			return _str;
		else if(*_str == '\0')
			return NULL;
		else
			_str++;
*/
	do {
		if(*str == c)
			return (char *)str;
	} while(*str++);

	return NULL;
}

// XXX: Check this later
// TODO: On second though, check this entire file
char * strrchr(const char *str, int c)
{
	char *ret = NULL;
	do {
		if(*str == c)
			ret = (char *)str;
	} while(*str++);

	return ret;
}

char *strcpy(char *dst, const char *src)
{
	char *_dst = dst;
	while((*_dst++ = *src++));
	return dst;
}

char *strncpy(char *dst, const char *src, size_t len)
{
	char *_dst = dst;

	while(((*_dst++ = *src++) != 0) && (--len != 0));

	return dst;
}

char * strdup(const char *str)
{
	char *new = kcalloc(strlen(str) + 1,1);
	if(new != NULL)
		strcpy(new, str);
	return new;
}

size_t strlen(const char *str)
{
	const char *tmp = str;
	while(*++tmp != '\0');
	return tmp - str;
}

char * strtok_r ( char * str, const char * delimiters, char **save )
{
	char *out;

	if(str == NULL)
		str = *save;

	while(strchr(delimiters,*str) != NULL)
	{
		if(*str == '\0')
		{
			*save = str;
			return NULL;
		}
		str++;
	}

	out = str;

	while(strchr(delimiters,*str) == NULL)
		str++;

	if(*str != '\0')
	{
		*str = '\0';
		*save = str + 1;
	} else {
		*save = str;
	}
	return out;
}

void *musl_memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

#ifdef __GNUC__

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LS >>
#define RS <<
#else
#define LS <<
#define RS >>
#endif

	typedef uint32_t __attribute__((__may_alias__)) u32;
	uint32_t w, x;

	for (; (uintptr_t)s % 4 && n; n--) *d++ = *s++;

	if ((uintptr_t)d % 4 == 0) {
		for (; n>=16; s+=16, d+=16, n-=16) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			*(u32 *)(d+8) = *(u32 *)(s+8);
			*(u32 *)(d+12) = *(u32 *)(s+12);
		}
		if (n&8) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			d += 8; s += 8;
		}
		if (n&4) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			d += 4; s += 4;
		}
		if (n&2) {
			*d++ = *s++; *d++ = *s++;
		}
		if (n&1) {
			*d = *s;
		}
		return dest;
	}

	if (n >= 32) switch ((uintptr_t)d % 4) {
	case 1:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		n -= 3;
		for (; n>=17; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+1);
			*(u32 *)(d+0) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+5);
			*(u32 *)(d+4) = (x LS 24) | (w RS 8);
			x = *(u32 *)(s+9);
			*(u32 *)(d+8) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+13);
			*(u32 *)(d+12) = (x LS 24) | (w RS 8);
		}
		break;
	case 2:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		n -= 2;
		for (; n>=18; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+2);
			*(u32 *)(d+0) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+6);
			*(u32 *)(d+4) = (x LS 16) | (w RS 16);
			x = *(u32 *)(s+10);
			*(u32 *)(d+8) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+14);
			*(u32 *)(d+12) = (x LS 16) | (w RS 16);
		}
		break;
	case 3:
		w = *(u32 *)s;
		*d++ = *s++;
		n -= 1;
		for (; n>=19; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+3);
			*(u32 *)(d+0) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+7);
			*(u32 *)(d+4) = (x LS 8) | (w RS 24);
			x = *(u32 *)(s+11);
			*(u32 *)(d+8) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+15);
			*(u32 *)(d+12) = (x LS 8) | (w RS 24);
		}
		break;
	}
	if (n&16) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&8) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&4) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&2) {
		*d++ = *s++; *d++ = *s++;
	}
	if (n&1) {
		*d = *s;
	}
	return dest;
#endif

	for (; n; n--) *d++ = *s++;
	return dest;
}

void *memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;
	for (; n; n--) *d++ = *s++;
	return dst;
	/*
	char *_src = (char *)src;
	char *end = (char *)((char *)src + size);

	while(_src <= end)
		*_dst++ = *_src++;

	return dst;*/
}

void *memset(void *dest, int val, size_t count)
{
	char *tmp = dest;
	char *end = (char *)(tmp + count);
	while(tmp != end)
		*tmp++ = val;

	return dest;
}

// XXX: This memcmp is from musl
int memcmp(const void *vl, const void *vr, size_t n)
{
	const unsigned char *l = vl, *r = vr;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l - *r : 0;
}