#ifndef C_OS_STDLIB_H
#define C_OS_STDLIB_H
#include <stddef.h>
#include <mm/liballoc.h>
// FIXME: why not just have them be named this to begin with?
#define calloc kcalloc
#define free kfree
#define malloc kmalloc

typedef struct {
    int quot;
    int rem;
} div_t;

div_t div(int numerator, int denominator);

//void *calloc(size_t num, size_t size);
//void free(void *ptr);

void exit(int e);

long strtol(const char * restrict str, char ** restrict ptr, int base);
#endif
