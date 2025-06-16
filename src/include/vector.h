#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct {
    void** data;
    size_t size;
    size_t capacity;
} Vector;

Vector* vector_new(size_t initial_capacity);
void vector_free(Vector* vector);
void vector_push(Vector* vector, void* element);
void* vector_pop(Vector* vector);
void* vector_get(Vector* vector, size_t index);
void vector_set(Vector* vector, size_t index, void* element);
size_t vector_size(Vector* vector);
void vector_clear(Vector* vector);

#endif // VECTOR_H 