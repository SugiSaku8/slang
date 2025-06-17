#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "vector.h"

// Type kinds
typedef enum {
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOLEAN,
    TYPE_FUNCTION,
    TYPE_VOID,
    TYPE_UNKNOWN
} TypeKind;

// Type structure
typedef struct Type {
    TypeKind kind;
    union {
        struct {
            struct Type* return_type;
            struct Type** parameter_types;
            size_t parameter_count;
        } function;
    } data;
} Type;

// Type system functions
Type* create_integer_type(void);
Type* create_float_type(void);
Type* create_string_type(void);
Type* create_boolean_type(void);
Type* create_function_type(Type* return_type, Type** parameter_types, size_t parameter_count);
Type* create_void_type(void);
Type* create_unknown_type(void);

bool types_are_compatible(Type* t1, Type* t2);
void free_type(Type* type);

#endif // TYPE_SYSTEM_H 