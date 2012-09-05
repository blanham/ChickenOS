#ifndef C_OS_STRING_H
#define C_OS_STRING_H
#include "types.h"

size_t strlen(const char *str);

void *memcpy(void *, const void *, size_t);
void *memset(void *dest, uint8_t val, size_t count);

char *strcpy(char *dst, const char *src);

int strcmp (const char * str1, const char * str2);
int strncmp ( const char * str1, const char * str2, size_t num);
char *strcat(char *dst, const char *src);
char * strchr(const char *str, int c);
char * strtok_r (char * str, const char * delimiters, char **save);
char * strtok (char * str, const char * delimiters);
char * strdup(const char *str);
#endif
