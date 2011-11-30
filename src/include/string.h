#ifndef C_OS_STRING_H
#define C_OS_STRING_H
#include <kernel/types.h>
int strlen(char *str);

void *memcpy(void *, const void *, size_t);


int strcmp (const char * str1, const char * str2);
#endif
