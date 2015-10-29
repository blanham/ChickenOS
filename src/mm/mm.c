/*	ChickenOS - mm/mm.c
 *	Thread level memory management
 */
#include <common.h>
#include <mm/vm.h>
#include <kernel/thread.h>
#include <stdio.h>

struct mm *mm_clone(struct mm *old)
{
	struct mm *new = kcalloc(sizeof(*new), 1);
//	new->pd = pagedir_new();
	new->pd = pagedir_copy(old->pd);
	new->regions = region_clone(old->regions);

	return new;
}

void mm_clear(struct mm *mm)
{
	(void)mm;
	//Iterate through regions and remove them
	//	reduce reference counts for physical pages
	//swap to a new pagedirectory and throw away everything else
}

void mm_free(struct mm *mm)
{
	(void)mm;
}

//Currently used in execve()
void mm_init(struct mm *mm)
{
	//This initial allocation is probably not needed,
	//but saves a page fault on initial stack setup
	void *user_stack = palloc();
	memset(user_stack, 0, 4096);

	mm->pd = pagedir_new();
	mm->regions = NULL;
	//XXX: Don't call this here
	pagedir_install(mm->pd);

	memregion_map_data(mm, PHYS_BASE - PAGE_SIZE, PAGE_SIZE,
			PROT_GROWSDOWN, MAP_GROWSDOWN | MAP_FIXED, user_stack);
	memregion_map_data(mm, HEAP_BASE, PAGE_SIZE,
			PROT_GROWSUP, MAP_PRIVATE | MAP_FIXED, NULL);

	//XXX: Don't call it here either
	pagedir_install(mm->pd);
}


