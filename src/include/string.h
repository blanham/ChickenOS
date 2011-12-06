#ifndef C_OS_STRING_H
#define C_OS_STRING_H
#include <kernel/types.h>
int strlen(char *str);

void *memcpy(void *, const void *, size_t);
void *memset(void *dest, uint8_t val, size_t count);

char *strcpy(char *dst, const char *src);

char *strcat(char *dst, const char *src);
int strcmp (const char * str1, const char * str2);
char * strchr(const char *str, int c);
int strncmp ( const char * str1, const char * str2, size_t num);
char * strtok_r (char * str, const char * delimiters, char **save);
char * strtok (char * str, const char * delimiters);
#endif
