#include "list.h"

#include <stdlib.h>
#include <string.h>

static const size_t MIN_CAPACITY = 64;

struct List {
	size_t capacity;
	size_t length;
	void **data;
};

static inline void list_ensure_capacity(List *list, size_t required) {
	if (list->capacity < required) {
		size_t new_capacity = list->capacity? list->capacity * 2 : MIN_CAPACITY;
		if (new_capacity < required) {
			new_capacity = required;
		}
		list->data = realloc(list->data, new_capacity * sizeof(void *));
		list->capacity = new_capacity;
	}
}

static inline void list_ensure_extra_items(List *list, int delta) {
	list_ensure_capacity(list, list->length + delta);
}

List *list_new() {
	List *list = malloc(sizeof(List));
	list->capacity = 0;
	list->length = 0;
	list->data = NULL;
	list_ensure_capacity(list, MIN_CAPACITY);
	return list;
}

void list_destroy(List *list) {
	free(list->data);
	free(list);
}

size_t list_length(const List *list) {
	return list->length;
}

void *list_nth(const List *list, size_t index) {
    if (index >= list->length) return NULL;
	return list->data[index];
}

bool list_index_of(const List *list, const void *value, size_t *out_i) {
    const size_t length = list->length;
    for (size_t i = 0; i < length; ++i) {
        if (list->data[i] == value) {
            if (out_i) {
                *out_i = i;
            }
            return true;
        }
    }
    return false;
}

void list_set(List *list, size_t index, void *item) {
    if (index >= list->length) return;
    list->data[index] = item;
}

size_t list_push(List *list, void *item) {
	list_ensure_extra_items(list, 1);

	size_t index = list->length;
	list->data[index] = item;
	list->length++;

	return index;
}

void *list_pop(List *list) {
	list_ensure_extra_items(list, -1);

	list->length--;
	return list->data[list->length];
}

void *list_remove(List *list, size_t index) {
    if (index >= list->length) return NULL;
	list_ensure_extra_items(list, -1);

	size_t length_bytes = (list->length - index) * sizeof(void *);
	void **dest = list->data + index;
	void **src = dest + 1;

	void *item = *dest;

	memmove(dest, src, length_bytes);
	list->length--;

	return item;
}

void list_clear(List *list) {
	list->length = 0;
}
