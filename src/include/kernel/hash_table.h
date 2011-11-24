#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

/* implementation of a simple hash table
 * 	-- thread safe functions as *_safe()
 *  -- resolve collisions by chaining
 * Direct comments, concerns, questions, bugs to:
 * kulesh [squiggly] isis.poly.edu
 */

#include <pthread.h>

#include "list.h"
#include "hash_function.h"

#define BUCKET_BITLEN	32

typedef int (*keycmp_ptr) (const void *, const void *, size_t);

struct hash_entry {
	struct list_head list;
	unsigned char *key;
	unsigned int keylen;
};

/* a hash_table contains buckets full of hash_entries (See above).
 * keycmp() is used to compare the keys of hash_entries
 */
struct hash_table {
	struct hash_entry *table;

	unsigned int buckets;
	pthread_mutex_t *bucket_locks;

	pthread_mutex_t lock;
	keycmp_ptr keycmp;

	/* private variables */
	unsigned int __ht_i;
	struct list_head *pos;
};

static inline int hash_table_bucket_lock(struct hash_table *t, unsigned int n)
{
	return (pthread_mutex_lock(&(t->bucket_locks[n])));
}

static inline int hash_table_bucket_unlock(struct hash_table *t, unsigned int n)
{
	return (pthread_mutex_unlock(&(t->bucket_locks[n])));
}

static inline int hash_table_lock(struct hash_table *t)
{
	return (pthread_mutex_lock(&(t->lock)));
}

static inline int hash_table_unlock(struct hash_table *t)
{
	return (pthread_mutex_unlock(&(t->lock)));
}

#ifdef EBUSY
static inline int hash_table_bucket_locked(struct hash_table *t, unsigned int n)
{
	return (pthread_mutex_trylock((t->bucket_locks[n])) == EBUSY);
}

static inline int hash_table_locked(struct hash_table *t)
{
	return (pthread_mutex_trylock(&(t->lock)) == EBUSY);
}
#else
static inline int hash_table_bucket_locked(struct hash_table *t, unsigned int n)
{
	return 0;
}

static inline int hash_table_locked(struct hash_table *t)
{
	return 0;
}
#endif

static inline int hash_table_hash_code(const struct hash_table *t,
				       const char *key, unsigned int len)
{

	return (__hash(key, len) % t->buckets);
}

static inline int hash_table_hash_code_safe(struct hash_table *t,
					    const char *key, unsigned int len)
{
	int n;

	hash_table_lock(t);
	n = __hash(key, len) % t->buckets;
	hash_table_unlock(t);

	return n;
}

static inline int hash_entry_init(struct hash_entry *e,
				  const unsigned char *str, unsigned int len)
{

	INIT_LIST_HEAD(&(e->list));

	if (str) {
		if ((e->key = (unsigned char *)malloc(len)) == NULL)
			return -1;
		memcpy(e->key, str, len);
		e->keylen = len;
	}
	return 0;
}

static inline void hash_entry_finit(struct hash_entry *e)
{
	if (e->key)
		free(e->key);
	e->keylen = 0;
}

static inline int hash_table_init(struct hash_table *h, unsigned int b,
				  keycmp_ptr keycmp)
{

	h->buckets = b;
	pthread_mutex_init(&(h->lock), NULL);

	if ((h->table =
	     (struct hash_entry *)malloc(sizeof(struct hash_entry) * b)) ==
	    NULL)
		return -1;

	if ((h->bucket_locks =
	     (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * b)) == NULL)
		return -1;

	for (--b; b != 0; --b) {
		hash_entry_init(&(h->table[b]), NULL, 0);
		pthread_mutex_init(&h->bucket_locks[b], NULL);
	}

	hash_entry_init(&(h->table[0]), NULL, 0);
	pthread_mutex_init(&h->bucket_locks[0], NULL);

	if (keycmp)
		h->keycmp = keycmp;
	else
		h->keycmp = &memcmp;

	return 0;
}

static inline void hash_table_finit(struct hash_table *h)
{

	if (h->table)
		free(h->table);
	h->buckets = 0;
}

/* insert_hash_table()
 * @h: &struct hash_table hash table to insert hash_entry into
 * @e: &struct hash_entry
 * Description: inserts @e into @h using @e->key as key. not thread-safe.
 */
void hash_table_insert(struct hash_table *h,
		       struct hash_entry *e,
		       const unsigned char *key, unsigned int len)
{
	unsigned int n;

	hash_entry_init(e, key, len);
	n = hash_table_hash_code(h, key, len);
	list_add(&(e->list), &(h->table[n].list));
}

/* insert_hash_table_safe()
 * @h: &struct hash_table hash table to insert hash_entry into
 * @e: &struct hash_entry
 * @key: use key to insert the hash_entry
 * @len: length of the key
 * Description: inserts @e into @h using @e->key as key. thread-safe.
 */
void hash_table_insert_safe(struct hash_table *h,
			    struct hash_entry *e,
			    const unsigned char *key, unsigned int len)
{
	unsigned int n;

	hash_entry_init(e, key, len);
	n = hash_table_hash_code_safe(h, key, len);

	hash_table_bucket_lock(h, n);
	list_add(&(e->list), &(h->table[n].list));
	hash_table_bucket_unlock(h, n);
}

/* hash_table_lookup_key()
 * @h: hash table to look into
 * @str: the key to look for
 * @len: length of the key
 * Description: looks up the hash table for the presence of key. 
 * Returns: returns a pointer to the hash_entry that matches the key. otherise returns NULL.
 * Notes: in the presence of duplicate keys the function returns the first hash_entry found.
 * 		  function is not safe from delections. 
 * 		  function is not thread safe. 
 */
struct hash_entry *hash_table_lookup_key(const struct hash_table *h,
					 const unsigned char *str,
					 unsigned int len)
{
	unsigned int key = hash_table_hash_code(h, str, len);
	struct hash_entry *tmp;
	struct list_head *pos;

	list_for_each(pos, &(h->table[key].list)) {
		tmp = list_entry(pos, struct hash_entry, list);

		if ((tmp->keylen == len)
		    && (h->keycmp(tmp->key, str, tmp->keylen) == 0))
			return tmp;
	}
	return NULL;
}

/* hash_table_lookup_key_safe()
 * @h: hash table to look into
 * @str: the key to look for
 * @len: length of the key
 * Description: looks up the hash table for the presence of key. 
 * Returns: returns a pointer to the hash_entry that matches the key. otherise returns NULL.
 * Notes: in the presence of duplicate keys the function returns the first hash_entry found.
 * 		  function is not safe from delections. 
 * 		  function is not thread safe. 
 */
struct hash_entry *hash_table_lookup_key_safe(struct hash_table *h,
					      const unsigned char *str,
					      unsigned int len)
{

	unsigned int key = hash_table_hash_code_safe(h, str, len);
	struct hash_entry *tmp;
	struct list_head *pos;

	hash_table_bucket_lock(h, key);

	list_for_each(pos, &(h->table[key].list)) {
		tmp = list_entry(pos, struct hash_entry, list);

		if (memcmp(tmp->key, str, tmp->keylen) == 0) {
			hash_table_bucket_unlock(h, key);
			return tmp;
		}
	}

	hash_table_bucket_unlock(h, key);
	return NULL;
}

/* same as hash_table_lookup_key() but this function takes a valid hash_entry as input.
 * a valid hash_entry is the one that has key, len set appropriately. in other words, a
 * hash_entry that is the output of hash_entry_init()
 */
static inline struct hash_entry *hash_table_lookup_hash_entry(const struct
							      hash_table *h,
							      const struct
							      hash_entry *e)
{
	return (hash_table_lookup_key(h, e->key, e->keylen));
}

/* same as hash_table_lookup_key_safe() but this function takes a valid hash_entry as 
 * input. a valid hash_entry is the one that has key, len set appropriately. in other 
 * words, a hash_entry that is the output of hash_entry_init()
 */
static inline struct hash_entry *hash_table_lookup_hash_entry_safe(struct hash_table
								   *h, const struct hash_entry
								   *e)
{
	return (hash_table_lookup_key_safe(h, e->key, e->keylen));
}

struct hash_entry *hash_table_del_key(struct hash_table *h, const char *str,
				      unsigned int len)
{
	struct hash_entry *e;

	if ((e = hash_table_lookup_key(h, str, len)) == NULL)
		return NULL;

	list_del_init(&(e->list));
	return e;
}

struct hash_entry *hash_table_del_key_safe(struct hash_table *h,
					   const char *str, unsigned int len)
{
	struct hash_entry *e;
	unsigned int n = hash_table_hash_code(h, str, len);

	hash_table_bucket_lock(h, n);
	if ((e = hash_table_lookup_key(h, str, len)) != NULL) {
		list_del_init(&(e->list));
		hash_table_bucket_unlock(h, n);
		return e;
	}

	hash_table_bucket_unlock(h, n);
	return NULL;
}

static inline struct hash_entry *hash_table_del_hash_entry(struct hash_table *h,
							   struct hash_entry *e)
{
	return (hash_table_del_key(h, e->key, e->keylen));
}

static inline struct hash_entry *hash_table_del_hash_entry_safe(struct
								hash_table *h,
								struct
								hash_entry *e)
{
	return (hash_table_del_key_safe(h, e->key, e->keylen));
}

/**
 * hash_entry - get the user data for this entry
 * @ptr:	the &struct hash_entry pointer
 * @type:	the type of the user data (e.g. struct my_data) embedded in this entry
 * @member:	the name of the hash_entry within the struct (e.g. entry)
 */
#define hash_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/*
 * @hentry: &struct hash_entry
 * @htable: &struct hash_table
 */
#define hash_table_for_each(hentry, htable)	\
	for	((htable)->__ht_i=0; ((htable)->__ht_i < (htable)->buckets); ++((htable)->__ht_i))	\
		for(((htable)->pos= (htable)->table[(htable)->__ht_i].list.next);		\
				((htable)->pos != &((htable)->table[(htable)->__ht_i].list)) &&	\
				((hentry) = ((struct hash_entry *)((char *)((htable)->pos)-(unsigned long)(&((struct hash_entry *)0)->list))) );	\
				(htable)->pos= (htable)->pos->next)

/*
 * @hentry: &struct hash_entry
 * @htable: &struct hash_table
 * @pos: &struct list_head
 * @hti: unsigned int
 */
#define hash_table_for_each_safe(hentry, htable, pos, hti)	\
	for	((hti)=0; ((hti) < (htable)->buckets); ++(hti))	\
		for(((pos)= (htable)->table[(hti)].list.next);		\
				((pos) != &((htable)->table[(hti)].list)) &&	\
				((hentry) = ((struct hash_entry *)((char *)((pos))-(unsigned long)(&((struct hash_entry *)0)->list))) );	\
				(pos)= (pos)->next)

#endif
