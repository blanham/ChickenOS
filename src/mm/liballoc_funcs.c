#include <kernel/common.h>
#include <kernel/interrupt.h>
#include <types.h>
#include <mm/vm.h>
#include <mm/liballoc.h>

enum intr_status alloc_intr_status;

//FIXME: Add locks here
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

