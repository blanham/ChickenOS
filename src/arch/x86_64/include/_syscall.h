#ifndef C_OS_ARCH_I386_SYSCALL_H
#define C_OS_ARCH_I386_SYSCALL_H

#define SYSCALL_0N(num) ({				\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num)			\
				   	);					\
					ret;})

#define SYSCALL_1N(num, arg0) ({		\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0) 		\
				   	);					\
					ret;})

#define SYSCALL_2N(num, arg0,arg1) ({	\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1)		\
										\
				   	);					\
					ret;})
#define SYSCALL_3N(num, arg0,arg1, arg2) ({	\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1),		\
					  "d" (arg2) 		\
				   	);					\
					ret;})
#define SYSCALL_4N(num, arg0,arg1, arg2, arg3) ({	\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1),		\
					  "d" (arg2), 		\
					  "S" (arg3)		\
				   	);					\
					ret;})

#endif
