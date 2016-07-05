#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct List List;

/**
 * \brief Creates a new list.
 * Creates a new empty list with default capacity.
 * @return The list.
 */
List *list_new();

/**
 * \brief Destroys a list.
 * Destroys a list, freeing the memory storing the content,
 * but not the content pointers stored in it.
 */
void list_destroy(List *list);

/**
 * Returns the length of a list (number of items contained in it).
 * @return The list length.
 */
size_t list_length(const List *list);

/**
 * \brief Gets the value at the nth index of the list.
 * This operation is equivalent to `arr[i]` for C arrays.
 * No bounds checking is performed.
 * @param index Index of the item in the list.
 * @return The item at the given position in the list.
 */
void *list_nth(const List *list, size_t index);

/**
 * \brief Returns the position of `value` in the list.
 * @param value Value to find.
 * @param out_i Destination of the index, or NULL if not needed.
 * @return Whether the value is in the list or not.
 */
bool list_index_of(const List *list,  const void *value, size_t *out_i);

/**
 * \brief Sets the value at the given index.
 * @param index Index to change
 * @param item New value for the index
 */
void list_set(List *list, size_t index, void *item);

/**
 * \brief Adds an item to the end of the list.
 * It can be subsequently accessed with
 * `list_nth(list, list_length(list) - 1)`.
 * @param item Value to be added to the list.
 * @return Index of the item just inserted.
 */
size_t list_push(List *list, void *item);

/**
 * \brief Pops the item at the end of the list.
 * Removes the item at the end of the list and returns it.
 * This function does not do any bounds checking,
 * popping an empty list is undefined!
 * @return The item that was removed.
 */
void *list_pop(List *list);

/**
 * \brief Removes the item at the given position.
 * Removes the item from the list, shifting all items
 * after it to the left to cover the gap.
 * @return The item that was removed.
 */
void *list_remove(List *list, size_t index);

/**
 * \brief Clears the list.
 * Removes all items from the list.
 */
void list_clear(List *list);

#endif
