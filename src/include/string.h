#ifndef C_OS_LIB_STRING_H
#define C_OS_LIB_STRING_H
#include <stddef.h>

size_t strlen(const char *str);

void *memcpy(void *, const void *, size_t);
void *memset(void *dest, int val, size_t count);
int memcmp (const void *s1, const void *s2, size_t n);

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t len);

int strcmp (const char * str1, const char * str2);
int strncmp ( const char * str1, const char * str2, size_t num);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t len);
char * strchr(const char *str, int c);
char * strtok_r (char * str, const char * delimiters, char **save);
char * strtok (char * str, const char * delimiters);
char * strdup(const char *str);
#endif
