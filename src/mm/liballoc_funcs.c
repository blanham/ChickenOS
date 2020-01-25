#include <stddef.h>
#include <chicken/interrupt.h>
#include <chicken/mm/liballoc.h>
#include <chicken/mm/vm.h>

enum intr_status alloc_intr_status = INTR_DISABLED;

// XXX: This may or may not need more advanced locking
int liballoc_lock() {
	alloc_intr_status = interrupt_disable();
	return 0;
}

int liballoc_unlock() {
	interrupt_set(alloc_intr_status);
	return 0;
}

void* liballoc_alloc(size_t pages) {
	return pallocn(pages);
}

int liballoc_free(void* ptr, size_t pages) {
	return pallocn_free(ptr, pages);
}