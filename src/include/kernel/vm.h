#ifndef C_OS_VM_H
#define C_OS_VM_H
#include <kernel/types.h>
#define PHYS_BASE 0xC0000000
#define V2P(p) (p - PHYS_BASE)
#define P2V(p) (p + PHYS_BASE)

extern unsigned int placement;

void paging_init();
void gdt_install();
void vm_init(uint32_t);
#endif
