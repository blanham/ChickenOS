//#include <stdint.h>
//#include <stdbool.h>
#ifndef C_OS_TYPES_H
#define C_OS_TYPES_H
typedef unsigned int	size_t;
typedef int				ssize_t;
typedef signed long int	off_t;
typedef signed   char 	int8_t;
typedef unsigned char 	uint8_t;
typedef signed   short 	int16_t;
typedef unsigned short 	uint16_t;
typedef signed   int 	int32_t;
typedef unsigned int 	uint32_t;
typedef signed long	long	int64_t;
typedef unsigned long long	uint64_t;
typedef uint32_t	    bool;
typedef uint32_t		uintptr_t;
typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;
#ifndef NULL
#define NULL 0
#endif
#define FALSE 0
#define TRUE 1
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#endif
