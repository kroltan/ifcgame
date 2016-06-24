#include "hashmap.h"

#include "list.h"

#include <stdlib.h>
#include <string.h>

static const size_t DEFAULT_BUCKET_COUNT = 32;

typedef struct {
    const void *key;
    void *value;
    hashmap_hash_t hash;
} HashMapNode;
typedef List *HashMapBucket;

struct HashMap {
    List *buckets; //Item: HashMapBucket
    HashMapComparator compare;
    HashMapHash hash;
};

static inline HashMapBucket hashmap_find_bucket(const HashMap *hmap, const void *key, bool create, hashmap_hash_t *out_hash) {
    hashmap_hash_t hash = hmap->hash(key);
    size_t bucket_idx = hash % list_length(hmap->buckets);
    *out_hash = hash;

    HashMapBucket bucket = list_nth(hmap->buckets, bucket_idx);
    if (!bucket && create) {
        bucket = list_new();
        list_set(hmap->buckets, bucket_idx, bucket);
    }

    return bucket;
}

static inline bool hashmap_get_node_index(const HashMap *hmap, hashmap_hash_t hash, HashMapBucket bucket, const void *key, size_t *out_idx) {
    size_t bucket_len = list_length(bucket);
    for (size_t i = 0; i < bucket_len; ++i) {
        HashMapNode *node = list_nth(bucket, i);
        if (node->hash == hash && hmap->compare(node->key, key) == 0) {
            *out_idx = i;
            return true;
        }
    }
    return false;
}


HashMap *hashmap_new(HashMapComparator comparator, HashMapHash hasher) {
    HashMap *hmap = malloc(sizeof(HashMap));
    hmap->buckets = list_new();
    hmap->compare = comparator;
    hmap->hash = hasher;

    for (size_t i = 0; i < DEFAULT_BUCKET_COUNT; ++i) {
        list_push(hmap->buckets, NULL);
    }

    return hmap;
}

int _strk_cmp(const void *a, const void *b) {
    return strcmp(a, b);
}
hashmap_hash_t _strk_hash(const void *key) {
    const char *keystr = key;
    size_t length = strlen(keystr);

    hashmap_hash_t hash = 0;

    for (size_t i = 0; i < length; ++i) {
        hash += keystr[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

HashMap *hashmap_new_str_key() {
    return hashmap_new(_strk_cmp, _strk_hash);
}

void hashmap_destroy(HashMap *hmap) {
    free(hmap->buckets);
    free(hmap);
}

void hashmap_set(HashMap *hmap, const void *key, void *value) {
    hashmap_hash_t hash;
    HashMapBucket bucket = hashmap_find_bucket(hmap, key, true, &hash);

    HashMapNode *node = malloc(sizeof(HashMapNode));
    node->key = key;
    node->value = value;
    node->hash = hash;

    list_push(bucket, node);
}

void *hashmap_get(const HashMap *hmap, const void *key) {
    void *value;
    if (hashmap_try_get(hmap, key, &value)) {
        return value;
    }
    return NULL;
}

bool hashmap_try_get(const HashMap *hmap, const void *key, void **out_value) {
    hashmap_hash_t hash;
    HashMapBucket bucket = hashmap_find_bucket(hmap, key, false, &hash);
    if (!bucket) return false;

    size_t node_i;
    if (hashmap_get_node_index(hmap, hash, bucket, key, &node_i)) {
        HashMapNode *node = list_nth(bucket, node_i);
        *out_value = node->value;
        return true;
    }

    return false;
}

void *hashmap_remove(HashMap *hmap, const void *key) {
    hashmap_hash_t hash;
    HashMapBucket bucket = hashmap_find_bucket(hmap, key, false, &hash);
    if (!bucket) return NULL;

    size_t node_i;
    if (hashmap_get_node_index(hmap, hash, bucket, key, &node_i)) {
        HashMapNode *node = list_remove(bucket, node_i);
        void *data = node->value;
        free(node);
        return data;
    }

    return NULL;
}

void hashmap_each(const HashMap *hmap, HashMapIterator callback) {
    const size_t bucket_count = list_length(hmap->buckets);
    for (size_t bucket_i = 0; bucket_i < bucket_count; ++bucket_i) {
        HashMapBucket bucket = list_nth(hmap->buckets, bucket_i);
        const size_t node_count = list_length(bucket);
        for (size_t node_i = 0; node_i < node_count; ++node_i) {
            HashMapNode *node = list_nth(bucket, node_i);
            callback(hmap, node->key, node->value);
        }
    }
}
