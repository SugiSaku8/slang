#ifndef SLANG_COMMON_H
#define SLANG_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// エラーコード
typedef enum {
    SLANG_SUCCESS = 0,
    SLANG_ERROR_LEXER = 1,
    SLANG_ERROR_PARSER = 2,
    SLANG_ERROR_TYPE = 3,
    SLANG_ERROR_RUNTIME = 4,
    SLANG_ERROR_INTERNAL = 5
} SlangError;

// 動的配列の実装
typedef struct {
    void* data;
    size_t size;
    size_t capacity;
    size_t element_size;
} Vector;

Vector* vector_create(size_t element_size);
void vector_destroy(Vector* vec);
bool vector_push(Vector* vec, const void* element);
void* vector_get(const Vector* vec, size_t index);
size_t vector_size(const Vector* vec);

#endif // SLANG_COMMON_H 