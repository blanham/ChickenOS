#ifndef C_OS_MM_PAGING_H
#define C_OS_MM_PAGING_H

void		paging_init(uint32_t mem_size);
void		page_invalidate(uintptr_t page);

uint32_t *	pagedir_alloc();
void		pagedir_free(uint32_t *);
void		pagedir_activate(uint32_t *pd);
uint32_t *	pagedir_clone(uint32_t * pd);
uintptr_t	pagedir_lookup(uint32_t * pd, uintptr_t virtual);
void		pagedir_map(uint32_t *pd, uintptr_t kvaddr, uintptr_t vaddr, bool rw);
void		pagedir_map_kernel(uintptr_t kvaddr, uintptr_t vaddr, bool rw);

#endif
