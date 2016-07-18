#include "hashmap.h"

#include "list.h"

#include <stdlib.h>
#include <string.h>

static const size_t DEFAULT_BUCKET_COUNT = 32;

typedef struct {
    void *key;
    void *value;
    HashMapHash hash;
} HashMapNode;

struct HashMap {
    List *buckets; //Item: List
    HashMapKeyAllocator keyAlloc;
    HashMapComparator compare;
    HashMapHasher hash;
};

HashMap *hashmap_new(HashMapKeyAllocator allocator, HashMapComparator comparator, HashMapHasher hasher) {
    HashMap *map = malloc(sizeof(*map));
    map->buckets = list_new();
    for (size_t i = 0; i < DEFAULT_BUCKET_COUNT; ++i) {
        list_push(map->buckets, list_new());
    }
    map->keyAlloc = allocator;
    map->compare = comparator;
    map->hash = hasher;

    return map;
}

void *_hashmap_strk_alloc(void *key, bool create) {
    if (create) {
        size_t str_len = strlen(key);
        char *new_key = malloc(str_len + 1);
        memcpy(new_key, key, str_len);
        new_key[str_len] = '\0';
        return new_key;
    } else {
        free(key);
        return NULL;
    }
}
HashMapHash _hashmap_strk_hash(const void *key) {
    const char *key_str = key;

    // http://stackoverflow.com/questions/7666509/hash-function-for-string
    HashMapHash hash = 5831;

    for (; *key_str; key_str++) {
        hash = ((hash << 5) + hash) + key_str[0];
    }

    return hash;
}
int _hashmap_strk_cmp(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}
HashMap *hashmap_new_str_key() {
    return hashmap_new(_hashmap_strk_alloc, _hashmap_strk_cmp, _hashmap_strk_hash);
}

void hashmap_destroy(HashMap *hmap) {
    size_t bucket_count = list_length(hmap->buckets);
    for (size_t bucket_i = 0; bucket_i < bucket_count; ++bucket_i) {
        List *bucket = list_nth(hmap->buckets, bucket_i);
        size_t node_count = list_length(bucket);
        for (size_t node_i = 0; node_i < node_count; ++node_i) {
            HashMapNode *node = list_nth(bucket, node_i);
            free(node->key);
            free(node);
        }
        list_destroy(bucket);
    }
    list_destroy(hmap->buckets);
}



List *_hashmap_bucket_for(const HashMap *hmap, HashMapHash hash) {
    size_t index = hash % list_length(hmap->buckets);
    return list_nth(hmap->buckets, index);
}

HashMapNode *_hashmap_find_node(const HashMap *hmap, const List *bucket, const void *key) {
    size_t length = list_length(bucket);
    for (size_t i = 0; i < length; ++i) {
        HashMapNode *current = list_nth(bucket, i);
        if (hmap->compare(current->key, key) == 0) {
            return current;
        }
    }
    return NULL;
}

void hashmap_set(HashMap *hmap, const void *key, void *value) {
    // safe to cast away key const when allocating
    void *new_key = hmap->keyAlloc((void *) key, true);
    HashMapHash hash = hmap->hash(new_key);

    List *bucket = _hashmap_bucket_for(hmap, hash);
    HashMapNode *node = _hashmap_find_node(hmap, bucket, new_key);
    if (!node) {
        node = malloc(sizeof(*node));
        node->key = new_key;
        node->hash = hash;
        list_push(bucket, node);
    }
    node->value = value;
}

void *hashmap_get(const HashMap *hmap, const void *key) {
    void *value;
    if (hashmap_try_get(hmap, key, &value)) {
        return value;
    }
    return NULL;
}

bool hashmap_try_get(const HashMap *hmap, const void *key, void **out_value) {
    const List *bucket = _hashmap_bucket_for(hmap, hmap->hash(key));
    HashMapNode *node = _hashmap_find_node(hmap, bucket, key);
    if (node) {
        *out_value = node->value;
        return true;
    }
    *out_value = NULL;
    return false;
}

void *hashmap_remove(HashMap *hmap, const void *key) {
    List *bucket = _hashmap_bucket_for(hmap, hmap->hash(key));
    HashMapNode *node = _hashmap_find_node(hmap, bucket, key);
    if (node) {
        void *value = node->value;

        hmap->keyAlloc(node->key, false);
        free(node);

        size_t index;
        if (list_index_of(bucket, node, &index)) {
            list_remove(bucket, index);
        }
        return value;
    }
    return NULL;
}

void hashmap_each(const HashMap *hmap, HashMapIterator callback) {
    size_t bucket_count = list_length(hmap->buckets);
    for (size_t bucket_i = 0; bucket_i < bucket_count; ++bucket_i) {
        List *bucket = list_nth(hmap->buckets, bucket_i);
        size_t node_count = list_length(bucket);
        for (size_t node_i = 0; node_i < node_count; ++node_i) {
            HashMapNode *node = list_nth(bucket, node_i);
            callback(hmap, node->key, node->value);
        }
    }
}
