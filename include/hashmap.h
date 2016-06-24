#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t hashmap_hash_t;
typedef struct HashMap HashMap;

typedef int (*HashMapComparator)(const void *a, const void *b);
typedef hashmap_hash_t (*HashMapHash)(const void *key);
typedef bool (*HashMapIterator)(const HashMap *hmap, const void *key, void *value);

HashMap *hashmap_new(HashMapComparator comparator, HashMapHash hasher);
HashMap *hashmap_new_str_key();
void hashmap_destroy(HashMap *hmap);

void hashmap_set(HashMap *hmap, const void *key, void *value);
void *hashmap_get(const HashMap *hmap, const void *key);
bool hashmap_try_get(const HashMap *hmap, const void *key, void **out_value);
void *hashmap_remove(HashMap *hmap, const void *key);

void hashmap_each(const HashMap *hmap, HashMapIterator callback);


#endif // HASHMAP_H
