#ifndef C_OS_STDLIB_H
#define C_OS_STDLIB_H
#include <stddef.h>
void *calloc(size_t num, size_t size);
void free(void *ptr);
long strtol(const char * restrict str, char ** restrict ptr, int base);
#endif
