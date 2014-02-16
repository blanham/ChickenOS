#ifndef C_OS_STDLIB_H
#define C_OS_STDLIB_H
#include <stddef.h>
#include <mm/liballoc.h>
#define calloc kcalloc
#define free kfree
#define malloc kmalloc

//void *calloc(size_t num, size_t size);
//void free(void *ptr);

void exit(int e);

long strtol(const char * restrict str, char ** restrict ptr, int base);
#endif
