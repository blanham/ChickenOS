#ifndef C_OS_VM_H
#define C_OS_VM_H
#include <kernel/types.h>
#include <multiboot.h>
#define PHYS_BASE 0xC0000000
#define V2P(p) (p - PHYS_BASE)
#define P2V(p) (p + PHYS_BASE)
typedef uint32_t phys_addr_t;
typedef uint32_t virt_addr_t;
typedef uint32_t * pagedir_t;

extern unsigned int placement;

void paging_init();
void gdt_install();
void vm_init(struct multiboot_info *mb);

void *pallocn(uint32_t count);
void *palloc();
void palloc_free(void *addr);
int pallocn_free(void *addr, int pages);

pagedir_t pagedir_new();

#endif
