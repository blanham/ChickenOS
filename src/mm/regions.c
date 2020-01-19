#include <common.h>
#include <stdint.h>
#include <chicken/thread.h>
#include <sys/tree.h>
#include <errno.h>
#include <mm/vm.h>
#include <fs/vfs.h>

uint8_t zero_page[PAGE_SIZE] __attribute__ ((aligned (PAGE_SIZE))) = {0};

// XXX: Why are these enums here?
enum {
	VERIFY_R,
	VERIFY_W
};

enum {
	REGION_R,
	REGION_W,
	REGION_COW
};


//XXX: Do the following in mm/regions.c:
//iterate through entries less than PHYS_BASE
//reduce reference counts
//(the frame allocator, will free pages that only had one reference)

void memregion_free(struct memregion *region)
{
	(void)region;
	//FIXME: Needs to be atomic
/*	atomic_fetch_sub(&region->frame->ref_count, 1);
	if(atomic_load(&region->frame->ref_count) == 0)
	{
		//close file if open
		if(region->file != NULL)
		{
			//vfs_close(file);
		}

		//We never
		if(region->cow == 1)
		{
			palloc_free(P2V(region->phys_addr));
		}
		//pagedir
		kfree(region);
	}*/
}

struct memregion *region_clone(struct memregion *original)
{
	struct memregion *new= NULL, *prev = NULL, *ret = NULL;

	for(struct memregion *p = original; p != NULL; p = p->next)
	{
		new = kcalloc(sizeof(*new), 1);
		memcpy(new, p, sizeof(*new));
		//printf("Cloning %x %x\n", p->addr_start, p->addr_end);
		if(ret == NULL)
			ret = new;
		else
			prev->next = new;
		// FIXME: Fix copy on write
		//p->cow = 1;
		//new->cow = 1;
		prev = new;
	}


	return ret;
}

struct memregion *memregion_new(struct mm *mm, uintptr_t address, size_t len, int prot, int flags)
{
	ASSERT(len > 0, "Len <= 0 passed to region_add");
	struct memregion *new = kcalloc(sizeof(*new), 1);

	new->pages = PAGE_COUNT(len);
	new->flags = flags;
	new->prot = prot;
	new->len = len;

	new->requested_start = address;
	new->requested_end = address + len;

	new->addr_start = address & PAGE_MASK;
	new->addr_end = new->addr_start + (new->pages * PAGE_SIZE);

	new->next = mm->regions;
	mm->regions = new;

	return new;
}


//TODO: Is this needed? This implementation is pretty brain dead at the moment
int memregion_map_data(struct mm *mm, uintptr_t address, size_t len, int prot, int flags,
		void *data)
{
	struct memregion *new = memregion_new(mm, address, len, prot, flags);

	if(data != NULL)
	{
		for (int i = 0; i < new->pages; i++) {
			uintptr_t offset = PAGE_SIZE*i;

	//XXX: This doesn't support setting protection levels at the moment, might have to
	//re-add flags to pagedir_map?
	//XXX: Nah, just don't bother to support PROT_EXEC, just PROT_READ/PROT_WRITE
	//		will still be POSIX compliant
			pagedir_map(mm->pd, (uintptr_t)data + offset, address + offset, true);
		}
		//pagedir_insert_pagen(mm->pd, (uintptr_t)data, address, 0x7, new->pages);
		pagedir_activate(mm->pd);
	}

	return 0;
}


int memregion_insert(struct mm *mm, void *data, uintptr_t address, uint8_t prot)
{
	if(data == NULL)
		data = palloc();
	address &= PAGE_MASK;

	//XXX: This doesn't support setting protection levels at the moment, might have to
	//re-add flags to pagedir_map
	(void)prot;
	//serial_printf("FUCK %x\n", address);
	pagedir_map(mm->pd, (uintptr_t)data, address, true);
	//serial_printf("FUCK\n");
	//pagedir_insert_page(mm->pd, (uintptr_t)data, address, prot);
	pagedir_activate(mm->pd);
	//serial_printf("FUCK\n");

	return 0;
}

static int regiongrowth(struct mm *mm, struct memregion *p, uintptr_t address)
{

	//Try to combine these two blocks
	if((p->prot == PROT_GROWSDOWN) && // (address + PAGE_SIZE >= p->addr_start) && // FIXME: Why did I comment this out?
			(address < p->addr_end))
	{
		//serial_printf("Growing down %x %x\n", address + PAGE_SIZE, address);
		p->addr_start -= PAGE_SIZE;

		// FIXME: This should take into account RLIMITs

		return memregion_insert(mm, NULL, address & PAGE_MASK, 0x7);
	} else

	if((p->prot == PROT_GROWSUP) && (address - PAGE_SIZE <= p->addr_end)
			&& (address >= p->addr_start))
	{
		//printf("Growing up %x %x end %x\n", address + PAGE_SIZE, address, p->addr_end);
		p->addr_end += PAGE_SIZE;

		return memregion_insert(mm, NULL, address & PAGE_MASK, 0x7);
	}

	return 1;
}

int memregion_map_file(struct mm *mm, uintptr_t address, size_t len, int prot,
		int flags, struct inode *inode, off_t offset, size_t size)
{
	ASSERT(inode != NULL, "NULL inode passed in");
	struct memregion *new = memregion_new(mm, address, len, prot, flags);
	new->inode = inode;
	new->file_offset = offset;
	new->file_size = size;
	//printf("offset %llx size %x\n", offset, size);
	return 0;
}

int load_page_from_file(struct mm *mm, struct memregion *p, uintptr_t address)
{
	ASSERT(p->inode != NULL, "Inode is a NULL pointer");




	off_t pages_from_start = (address & PAGE_MASK)  - p->addr_start;
	off_t file_offset = p->file_offset + pages_from_start;

	void *new = palloc();
	memset(new, 0, PAGE_SIZE);
	//printf("LLL %p\n", address);
	if (pages_from_start < p->file_size) {


		size_t len = PAGE_SIZE;
		//if (p->file_size - file_offset < PAGE_SIZE)
		//	len = p->file_size - file_offset;
	//	if (p->file_size - file_offset < PAGE_SIZE)
	//		len = p->file_size - (size_t)file_offset;
	//	
		//printf("LEN: %i\n", len);
		void *temp = palloc();
		size_t ret = p->inode->read(p->inode, temp, len, file_offset);

		size_t f = PAGE_SIZE;
		if (p->file_size - pages_from_start < PAGE_SIZE)
			f= p->file_size - pages_from_start;
			//printf("TTT %x\n", p->file_size - pages_from_start);
		memcpy(new,temp, f);
		(void)ret;

		//printf("Address: %p start: %p\n", address, p->addr_start);
		//printf("Loading from file %8llx %8x %6llx %8x ret: %4x other: %i\n", pages_from_start, p->file_size, file_offset, len, ret, p->file_size - file_offset);
	}
	memregion_insert(mm, new, address, 0x7);

	return 0;
}


int load_page_from_files(struct mm *mm, struct memregion *p, uintptr_t address)
{
	ASSERT(p->inode != NULL, "Inode is a NULL pointer");

	uint32_t page = address & PAGE_MASK;
	uint32_t offset = (page - p->addr_start);

	//FIXME: This should be a frame, not a bare page from palloc()
	void *new = palloc();

	if (offset <= p->file_size) {
		uint32_t off = p->file_offset + offset;
		uint32_t len = p->file_size - offset;
		//printf("Loading from file %x %x %x %x\n",page, offset, len, off);
		p->inode->fs->ops->read(p->inode, new, PAGE_SIZE, off);
		if (len < PAGE_SIZE) {
			memset(new + len, 0, PAGE_SIZE - len);
		}
	} else {
		memset(new, 0, PAGE_SIZE);
	}

	memregion_insert(mm, new, address, 0x7);

	return 0;
}

int memregion_cow(struct mm *mm, struct memregion *p, uintptr_t address)
{
	(void)p;
	PANIC("COW IS BROKEN\n");
	{
				phys_addr_t phys = pagedir_lookup(mm->pd, address);
				//XXX: this is bad and I should feel bad
				void *new = palloc();
				if(phys == 0)
					memset(new, 0, PAGE_SIZE);
				else
					memcpy(new, (void *)P2V(phys), PAGE_SIZE);
				address &= PAGE_MASK;

				memregion_insert(mm, new, address, 0x7);
				p->cow = 0;
				return 0;
			}
	return 1;
}

int memregion_fault(struct mm *mm, uintptr_t address, int prot)
{
	//TODO: Use AVL tree instead

	for(struct memregion *p = mm->regions; p != NULL; p = p->next) {
		//printf("KBs %i\n", (p->addr_end - p->addr_start + 1) / 1024);
		//printf("Ap->addr %x end %x prot %x address %x offset %llx\n", p->addr_start, p->addr_end, prot, address, p->file_offset);
		//TODO: actually check protection
		(void)prot;
	if (address < 4096)
		return 1;
		if(address >= p->addr_start && address < p->addr_end) {
			if(p->cow)
				return memregion_cow(mm, p, address);

			if(p->flags & MAP_FIXED)
				return memregion_insert(mm, NULL, address, 0x7);

			if((p->flags & MAP_FILE) == 0)
				return load_page_from_file(mm, p, address);

			PANIC("Address fell within a mapping, but no match found\n");
		}
		if(regiongrowth(mm, p, address) == 0)
			return 0;
	}
	return 1;
}

//FIXME: This is a dummy for now until we use a tree to lookup regions
//TODO:  Can probably just have this call memregion_fault, since if it's
//       valid but not loaded in we might as well load it in instead of
//       faulting it in
int verify_pointer(const void *ptr, size_t len, int rw)
{
//	thread_t *cur = thread_current();
	//size_t count = 0;
	//struct memregion *p = NULL;
	(void)rw;
	(void)len;

	if((uintptr_t)ptr > PHYS_BASE)
		return -EFAULT;

	return 0;
	/*
	while (count < len)
	{
		//lookup in tree

		if(p == NULL)
			return -EFAULT;
		//Here we do stuff
		if(p->present)
		{}//g2g
		if(!p->present && p->file != NULL)
		{}//map in file pages

		count += p->len;
		ptr += count;
	}

	return 0;*/
}
