#ifndef C_OS_ARCH_X86_64_COMMON_H
#define C_OS_ARCH_X86_64_COMMON_H
#include <stdint.h>

#define stackpointer_get(x)  asm volatile ("mov %%rsp, %0": "=m"(x) )

static inline void kernel_halt()
{
	while(1)
		asm volatile ("hlt");
}

static inline void print_user_trace (uint64_t *rbp)
{
      uint64_t *rip;
      asm volatile ("mov %%rbp, %0" : "=r" (rbp)); // Start with the current EBP value.
      while (rbp) {
        rip = rbp+1;
        printf ("[0x%x], ", *rip);
        rbp = (uint64_t*) *rbp;	
	//	if((uintptr_t)rbp < 0xC0000000)
		//	break;
      }
}

static inline void print_stack_trace ()
{
      uint64_t *rbp, *rip;
      asm volatile ("mov %%rbp, %0" : "=r" (rbp)); // Start with the current EBP value.
      while (rbp) {
        rip = rbp+1;
        printf ("[0x%x], ", *rip);
        rbp = (uint64_t*) *rbp;	
	//	if((uintptr_t)ebp < 0xC0000000)
		//	break;
      }
}

#endif