#ifndef C_OS_ARCH_ARM_SYSCALL_H
#define C_OS_ARCH_ARM_SYSCALL_H
#define SYSCALL_0N(num) ({				\
					int ret = 0;		\
/*	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num)			\
				   	);	*/				\
					ret;})



#define SYSCALL_3N(num, arg0,arg1, arg2) ({	\
					int ret = 0;			\
					(void)num; (void)arg0; (void)arg1; (void)arg2;\
/*	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1),		\
					  "d" (arg2) 		\
				   	);					\*/\
					ret;})


#endif
