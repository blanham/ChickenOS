#define PHYS_BASE 0xC0000000
#define V2P(p) (p - PHYS_BASE)
#define P2V(p) (p + PHYS_BASE)

void paging_init();
void gdt_install();
void vm_init();
