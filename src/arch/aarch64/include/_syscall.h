#ifndef C_OS_ARCH_AARCH64_SYSCALL_H
#define C_OS_ARCH_AARCH64_SYSCALL_H

#define SYSCALL_0N(num) ({				\
	register long x0 asm("x0");			\
	register long x8 asm("x8") = (long)num;\
	asm volatile (	"svc 0"				\
				   	: "=r"(x0) 			\
				   	: "r" (x8),			\
					  "0" (x0) 			\
					  : "memory", "cc"	\
				   	);					\
					x0;})
#define SYSCALL_1N(num, arg0) ({		\
	register long x0 asm("x0") = (long)arg0;\
	register long x8 asm("x8") = (long)num;	\
	asm volatile (	"svc 0"				\
				   	: "=r"(x0) 			\
				   	: "r" (x8),			\
					  "0" (x0)			\
					  : "memory", "cc"	\
				   	);					\
					x0;})
#define SYSCALL_2N(num, arg0, arg1) ({	\
	register long x0 asm("x0") = (long)arg0;\
	register long x8 asm("x8") = (long)num;	\
	register long x1 asm("x1") = (long)arg1;\
	asm volatile (	"svc 0"				\
				   	: "=r"(x0) 			\
				   	: "r" (x8),			\
					  "0" (x0), 		\
					  "r" (x1)			\
					  : "memory", "cc"	\
				   	);					\
					x0;})
#define SYSCALL_3N(num, arg0, arg1, arg2) ({\
	register long x0 asm("x0") = (long)arg0;\
	register long x8 asm("x8") = (long)num;	\
	register long x1 asm("x1") = (long)arg1;\
	register long x2 asm("x2") = (long)arg2;\
	asm volatile (	"svc 0"				\
				   	: "=r"(x0) 			\
				   	: "r" (x8),			\
					  "0" (x0), 		\
					  "r" (x1),			\
					  "r" (x2) 			\
					  : "memory", "cc"	\
				   	);					\
					x0;})
#define SYSCALL_4N(num, arg0, arg1, arg2, arg3) ({	\
	register long x0 asm("x0") = (long)arg0;\
	register long x8 asm("x8") = (long)num;	\
	register long x1 asm("x1") = (long)arg1;\
	register long x2 asm("x2") = (long)arg2;\
	register long x3 asm("x3") = (long)arg3;\
	asm volatile (	"svc 0"				\
				   	: "=r"(x0) 			\
				   	: "r" (x8),			\
					  "0" (x0), 		\
					  "r" (x1),			\
					  "r" (x2), 		\
					  "r" (x3)			\
					  : "memory", "cc"	\
				   	);					\
					x0;})
#endif
