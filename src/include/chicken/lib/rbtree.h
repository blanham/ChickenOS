#ifndef C_OS_LIB_RBTREE_H
#define C_OS_LIB_RBTREE_H
#include <stdint.h>

typedef struct __rbtree rbtree_t;

rbtree_t *rbtree_alloc();
void rbtree_free(rbtree_t *tree);
void rbtree_insert(rbtree_t *tree, uint64_t key, uintptr_t value);
void rbtree_insert_ptr(rbtree_t *tree, uint64_t key, void *ptr);
void rbtree_insert_range(rbtree_t *tree, uint64_t key, size_t range, uintptr_t value);
uintptr_t rbtree_search(rbtree_t *tree, uint64_t key);
uintptr_t rbtree_search_range(rbtree_t *tree, uint64_t key);
uintptr_t rbtree_search_range2(rbtree_t *tree, uint64_t key, size_t range);
void rbtree_update_range(rbtree_t *tree, uint64_t key, size_t range);
void rbtree_dump(rbtree_t *tree);

#endif
