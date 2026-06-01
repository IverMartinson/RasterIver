#ifndef KT_FUNCTIONS_H
#define KT_FUNCTIONS_H

#include <stdlib.h>

// allocate and return a new KT array
void** KT_new_array(size_t element_count);

// add a void* to the end of the array
void KT_append(void*** array, void* pointer);

// add a void* to the start of the array
void KT_prepend(void*** array, void* pointer);

// add a void* to the left of an index in the array
void KT_insert_left(void*** array, void* pointer, size_t index);

// add a void* to the right of an index in the array
void KT_insert_right(void*** array, void* pointer, size_t index);

// remove a void* from the array (this shifts all data following over, unlike KT_swap_pop)
void KT_drop(void*** array, size_t index);

// remove a void* from the array (unlike KT_drop, it does not move following data over, but instead changes the order of the array)
void KT_swap_pop(void*** array, size_t index);

// returns the number of elements currently in array
size_t KT_get_size(void*** array);

#endif