#ifndef C_OS_STDDEF_H
#define C_OS_STDDEF_H

typedef long ptrdiff_t;

#ifndef _SIZE_T
typedef unsigned int size_t;
#define _SIZE_T
#endif

typedef int wint_t;
#ifndef NULL
#define NULL 0
#endif
#endif
