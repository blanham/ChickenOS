#ifndef C_OS_MM_PAGING_H
#define C_OS_MM_PAGING_H
#define PDE_MASK	0xffc00000
#define PDE_SHIFT 	22
#define PDE_P		0x00000001
#define PDE_RW		0x00000002
#define PDE_USER	0x00000004
#define PDE_WTHRU	0x00000008
#define	PDE_DCACHE	0x00000010
#define PDE_ACCESED	0x00000020
#define PDE_DIRTY	0x00000040
#define PDE_GLOBAL	0x00000080
#define PDE_USER0	0x00000100
#define PDE_USER1	0x00000200
#define PDE_USER2	0x00000400

#define PTE_MASK	0x003ff000
#define PTE_SHIFT 	12
#define PTE_P		PDE_P		
#define PTE_RW		PDE_RW		
#define PTE_USER	PDE_USER	
#define PTE_WTHRU	PDE_WTHRU	
#define	PTE_DCACHE	PDE_DCACHE	
#define PTE_ACCESED	PDE_ACCESED	
#define PTE_DIRTY	PDE_DIRTY	
#define PTE_USER0	PDE_USER0
#define PTE_USER1	PDE_USER1
#define PTE_USER2	PDE_USER2

#define PDE_COUNT 1024
#define PTE_COUNT 1024
#define PTE_SIZE  4096
#define PDE_SIZE  4096
#define KERNEL_PDE_START 768
typedef uint32_t * pagedir_t;

void paging_init(uint32_t mem_size);
pagedir_t pagedir_new();
pagedir_t pagedir_clone(pagedir_t pd);
pagedir_t pagedir_copy(pagedir_t pd);
void page_invalidate(uintptr_t page);

void pagedir_install(uint32_t *pd);

void pagedir_insert_page(pagedir_t pd, 
	virt_addr_t phys, virt_addr_t virt,uint8_t flags);
void pagedir_insert_pagen(pagedir_t pd, virt_addr_t kvirt, 
	virt_addr_t uvirt,uint8_t flags, int n);

void pagedir_insert_page_physical(pagedir_t pd, phys_addr_t kphys, 
	virt_addr_t uvirt,uint8_t flags);
void pagedir_insert_pagen_physical(pagedir_t pd, phys_addr_t kphys, 
	virt_addr_t uvirt,uint8_t flags, int n);

phys_addr_t pagedir_lookup(pagedir_t pd, virt_addr_t virtual);
#endif

