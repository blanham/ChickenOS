#include <stdbool.h>
#include <stdlib.h>
#include <chicken/common.h>
#include <chicken/lib/rbtree.h>

typedef struct _node {
	struct _node *l, *r;
	uint64_t key, max;
	uintptr_t value;
	bool red;
} rbnode_t;

typedef struct __rbtree {
	rbnode_t *head; // head->r is the actual root of the tree (see Sedgwick C book pg 203 for details)
	rbnode_t *zero;
} rbtree_t;

typedef rbtree_t intervaltree_t;

rbtree_t *rbtree_alloc() {
	rbtree_t *ret = kcalloc(sizeof(*ret), 1);
	ret->head = kcalloc(sizeof(*ret->head), 1);
	ret->zero = kcalloc(sizeof(*ret->zero), 1);
	ret->zero->l = ret->zero; ret->zero->r = ret->zero; ret->zero->value = -1;
	ret->head->r = ret->zero; ret->head->l = NULL;
	return ret;
}

struct nodeinfo {
	rbnode_t *x, *parent, *grandparent, *greatgrandparent;
};

static rbnode_t *rbtree_rotate(rbnode_t *y, uint64_t key) {
	rbnode_t *gc = NULL;
	rbnode_t *c = (key < y->value) ? y->l : y->r;

	if (key < c->value) {
		gc = c->l; c->l = gc->r; gc->r = c;
	} else {
		gc = c->r; c->r = gc->l; gc->l = c;
	}

	if (key < y->value) y->l = gc; else y->r = gc;

	return gc;
}

static void rbtree_split(rbtree_t *tree, struct nodeinfo *state, uint64_t key) {
	state->x->red = true; state->x->l->red = false; state->x->r->red = false;

	if (state->parent->red) {
		state->grandparent->red = true;
		if ((key < state->grandparent->key) != (key < state->parent->key)) state->parent = rbtree_rotate(state->grandparent, key);
		state->x = rbtree_rotate(state->greatgrandparent, key);
		state->x->red = false;
	}

	tree->head->r->red = false;
}

void rbtree_insert_range(rbtree_t *tree, uint64_t key, size_t range, uintptr_t value) {
	struct nodeinfo info = {
		.x = tree->head, .parent = tree->head, .grandparent = tree->head, .greatgrandparent = NULL
	};

	//printf("farts %p %llx %x %x\n", tree, key, range, value);
	while (info.x != tree->zero) {
		info.greatgrandparent = info.grandparent; info.grandparent = info.parent; info.parent = info.x;
		info.x = (key <  info.x->value) ? info.x->l : info.x->r;
		if (info.x->l->red && info.x->r->red) rbtree_split(tree, &info, key);
	}

	rbnode_t *ret = kcalloc(sizeof *ret, 1);
	ret->key = key;
	ret->max = key + range;
	ret->value = value;
	ret->l = tree->zero;
	ret->r = tree->zero;

	if (key < info.parent->key) info.parent->l = ret; else info.parent->r = ret;

	info.x = ret;

	rbtree_split(tree, &info, key);

	//rbtree_dump(tree);
}

void rbtree_insert(rbtree_t *tree, uint64_t key, uintptr_t value) {
	rbtree_insert_range(tree, key, 0, value);
}

void rbtree_insert_ptr(rbtree_t *tree, uint64_t key, void *ptr) {
	rbtree_insert_range(tree, key, 0, (uintptr_t)ptr);
}

uintptr_t rbtree_search(rbtree_t *tree, uint64_t key) {
	rbnode_t *x = tree->head->r;
	tree->zero->key = key; // See Sedgwick page 203
	while (key != x->key && x != tree->zero) 
		x = (key < x->key) ? x->l : x->r;
	return x->value;
}

// TODO: Give these better names
uintptr_t rbtree_search_range(rbtree_t *tree, uint64_t key) {
	rbnode_t *x = tree->head->r;
	//tree->zero->key = key; // See Sedgwick page 203
	while (!((key >= x->key) && (key < x->max)) && x != tree->zero)
		x = (key < x->key) ? x->l : x->r;
	return x->value;
}

uintptr_t rbtree_search_range2(rbtree_t *tree, uint64_t key, size_t range) {
	rbnode_t *x = tree->head->r;
	//tree->zero->key = key; // See Sedgwick page 203
	while (!((key >= x->key) && (key < x->max + range)) && x != tree->zero)
		x = (key < x->key) ? x->l : x->r;
	return x->value;
}

uintptr_t rbtree_search_range3(rbtree_t *tree, uint64_t key, size_t range) {
	rbnode_t *x = tree->head->r;
	//tree->zero->key = key; // See Sedgwick page 203
	while (!((key >= x->key-range) && (key < x->max)) && x != tree->zero)
		x = (key < x->key) ? x->l : x->r;
	return x->value;
}



void rbtree_update_range(rbtree_t *tree, uint64_t key, size_t range) {
	rbnode_t *x = tree->head->r;
	tree->zero->key = key; // See Sedgwick page 203
	while (!((key >= x->key) && (key < x->max)))
		x = (key < x->key) ? x->l : x->r;
	x->max = key+range;
}

static void rbtree_dumper(rbtree_t *tree, rbnode_t *x) {

	if (x != tree->zero) {
		rbtree_dumper(tree, x->l);
		serial_printf("%p %llx %x %x\n", x, x->key, x->value, x->max);
		rbtree_dumper(tree, x->r);
	}
}

void rbtree_dump(rbtree_t *tree) {
	printf("Tree: %p\n", tree);
	rbtree_dumper(tree, tree->head->r);
}
