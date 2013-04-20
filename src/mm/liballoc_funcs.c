#include <kernel/common.h>
#include <kernel/interrupt.h>
#include <kernel/types.h>
#include <kernel/vm.h>
#include <mm/liballoc.h>

//FIXME: Add locks here
int liballoc_lock() {

        return 0;
}

int liballoc_unlock() {
        return 0;
}

void* liballoc_alloc(size_t pages) {
        return pallocn(pages);
}

int liballoc_free(void* ptr, size_t pages) {
        return pallocn_free(ptr, pages);
}

