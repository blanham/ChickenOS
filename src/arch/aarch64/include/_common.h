#ifndef C_OS_ARCH_AARCH64_COMMON_H
#define C_OS_ARCH_AARCH64_COMMON_H

#define stackpointer_get(x)  {x = 0;}

//asm volatile ("mov %%r0, %0": "=m"(x) )

static inline void kernel_halt()
{
	while(1)
		;//asm volatile ("hlt");
}

static inline void print_user_trace (uint32_t *ebp)
{
	(void)ebp;
	/*  
      uint32_t *eip;
      asm volatile ("mov %%ebp, %0" : "=r" (ebp)); // Start with the current EBP value.
      while (ebp)
      {
        eip = ebp+1;
        printf ("[0x%x], ", *eip);
        ebp = (uint32_t*) *ebp;	
	//	if((uintptr_t)ebp < 0xC0000000)
		//	break;
      }*/
}

static inline void print_stack_trace ()
{
      /* uint32_t *ebp, *eip;
      asm volatile ("mov %%ebp, %0" : "=r" (ebp)); // Start with the current EBP value.
      while (ebp)
      {
        eip = ebp+1;
        printf ("[0x%x], ", *eip);
        ebp = (uint32_t*) *ebp;	
	//	if((uintptr_t)ebp < 0xC0000000)
		//	break;
      }*/
}

#endif
