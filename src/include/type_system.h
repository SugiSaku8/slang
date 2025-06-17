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
    TYPE_UNKNOWN,
    TYPE_NAMED,
    TYPE_ARRAY,
    TYPE_TUPLE,
    TYPE_VECTOR,
    TYPE_MATRIX,
    TYPE_TENSOR,
    TYPE_QUATERNION,
    TYPE_COMPLEX
} TypeKind;

// Type structure
typedef struct Type {
    TypeKind kind;
    union {
        struct {
            char* name;
        } named;
        struct {
            struct Type* return_type;
            struct Type** parameter_types;
            size_t parameter_count;
        } function;
        struct {
            struct Type* element_type;
        } array;
        struct {
            struct Type** types;
            size_t type_count;
        } tuple;
        struct {
            struct Type* element_type;
            size_t dimension;
        } vector;
        struct {
            struct Type* element_type;
            size_t rows;
            size_t columns;
        } matrix;
        struct {
            struct Type* element_type;
            size_t* dimensions;
            size_t dimension_count;
        } tensor;
        struct {
            struct Type* element_type;
        } quaternion;
        struct {
            struct Type* element_type;
        } complex;
    } data;
} Type;

// Field structure for type definitions
typedef struct {
    char* name;
    Type* type;
} Field;

// Type system functions
Type* create_integer_type(void);
Type* create_float_type(void);
Type* create_string_type(void);
Type* create_boolean_type(void);
Type* create_function_type(Type* return_type, Type** parameter_types, size_t parameter_count);
Type* create_void_type(void);
Type* create_unknown_type(void);
Type* create_array_type(Type* element_type);
Type* create_tuple_type(Type** types, size_t type_count);

bool types_are_compatible(Type* t1, Type* t2);
void free_type(Type* type);

#endif // TYPE_SYSTEM_H 