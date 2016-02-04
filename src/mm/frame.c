/* ChickenOS - mm/frame.c
 * Frame allocator
 * Keeps track of page reference counts
 *
 */
#include <mm/vm.h>

struct frame *frames;

void frame_init(uintptr_t mem_size)
{
	int frame_count = mem_size / PAGE_SIZE;
	size_t frame_in_memory_size = sizeof(struct frame) * frame_count;

	frames = palloc_len(frame_in_memory_size);
}

struct frame *frame_get(void *ptr)
{
	uint32_t index = (uintptr_t)ptr >> PAGE_SHIFT;
	struct frame *ret = &frames[index];

	if(atomic_fetch_add(&ret->ref_count, 1) == 0) {
		ret->virt_ptr = ptr;
		ret->phys_addr = (uintptr_t)V2P(ptr);
	}

	return ret;
}

void frame_put(struct frame *frame)
{
	if(atomic_fetch_sub(&frame->ref_count, 1) == 1) {
		palloc_free(frame->virt_ptr);
	}
}

void *palloc_user()
{
	struct frame *new = frame_get(palloc());
	return new->virt_ptr;
}
