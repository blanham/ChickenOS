#ifndef C_OS_MM_FRAME_H
#define C_OS_MM_FRAME_H
#include <stdatomic.h>
#include <stdint.h>

union address {
	uintptr_t address;
	void *ptr;
};

typedef struct frame {
	union {
		uintptr_t phys_addr;
		void *phys_ptr;
	};
	union {
		uintptr_t virt_addr;
		void *virt_ptr;
	};
	size_t blocksize;
	union {
		//
		// if mapped
		// struct inode * parent?
		// if swapped
		struct swapdata{
		//	dev_t device;
			size_t block;
		} swapdata;
	};
	// hmm, then you wouldn't need to store block numbers in the frame
	// add lock/semaphore/waitqueue
	atomic_int ref_count;
} frame_t;

/* mm/frame.c */
void	 frame_init(uintptr_t mem_size);
frame_t *frame_get(void *ptr);
void	 frame_put(struct frame *frame);
void *	 palloc_user();


#endif