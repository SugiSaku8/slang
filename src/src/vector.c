#include <stdlib.h>
#include <string.h>
#include "../include/vector.h"

#define VECTOR_INITIAL_CAPACITY 16
#define VECTOR_GROWTH_FACTOR 2

Vector* vector_new(size_t initial_capacity) {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    if (!vector) return NULL;

    vector->capacity = initial_capacity > 0 ? initial_capacity : VECTOR_INITIAL_CAPACITY;
    vector->size = 0;
    vector->data = (void**)malloc(sizeof(void*) * vector->capacity);
    
    if (!vector->data) {
        free(vector);
        return NULL;
    }

    return vector;
}

void vector_free(Vector* vector) {
    if (!vector) return;
    free(vector->data);
    free(vector);
}

static bool vector_resize(Vector* vector, size_t new_capacity) {
    void** new_data = (void**)realloc(vector->data, sizeof(void*) * new_capacity);
    if (!new_data) return false;

    vector->data = new_data;
    vector->capacity = new_capacity;
    return true;
}

void vector_push(Vector* vector, void* element) {
    if (!vector) return;

    if (vector->size >= vector->capacity) {
        if (!vector_resize(vector, vector->capacity * VECTOR_GROWTH_FACTOR)) {
            return;
        }
    }

    vector->data[vector->size++] = element;
}

void* vector_pop(Vector* vector) {
    if (!vector || vector->size == 0) return NULL;
    return vector->data[--vector->size];
}

void* vector_get(Vector* vector, size_t index) {
    if (!vector || index >= vector->size) return NULL;
    return vector->data[index];
}

void vector_set(Vector* vector, size_t index, void* element) {
    if (!vector || index >= vector->size) return;
    vector->data[index] = element;
}

size_t vector_size(Vector* vector) {
    return vector ? vector->size : 0;
}

void vector_clear(Vector* vector) {
    if (!vector) return;
    vector->size = 0;
} 