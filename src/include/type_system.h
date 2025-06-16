#ifndef SLANG_TYPE_SYSTEM_H
#define SLANG_TYPE_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Forward declarations
typedef struct Vector Vector;

// Type kinds
typedef enum {
    TYPE_UNIT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_TUPLE,
    TYPE_VECTOR,
    TYPE_MATRIX,
    TYPE_TENSOR,
    TYPE_QUATERNION,
    TYPE_COMPLEX,
    TYPE_FUNCTION,
    TYPE_POINTER,
    TYPE_NAMED
} TypeKind;

// Type structure
typedef struct Type {
    TypeKind kind;
    union {
        struct {
            struct Type* element_type;
        } array;
        struct {
            Vector* types;
        } tuple;
        struct {
            size_t dimension;
            struct Type* element_type;
        } vector;
        struct {
            size_t rows;
            size_t columns;
            struct Type* element_type;
        } matrix;
        struct {
            Vector* dimensions;
            struct Type* element_type;
        } tensor;
        struct {
            struct Type* element_type;
        } quaternion;
        struct {
            struct Type* element_type;
        } complex;
        struct {
            Vector* params;
            struct Type* return_type;
            uint32_t* priority;
        } function;
        struct {
            struct Type* inner_type;
        } pointer;
        struct {
            char* name;
        } named;
    };
} Type;

// Type system functions
Type* type_new(TypeKind kind);
void type_free(Type* type);
bool type_is_vector(const Type* type);
bool type_is_matrix(const Type* type);
bool type_is_tensor(const Type* type);
bool type_is_quaternion(const Type* type);
bool type_is_complex(const Type* type);
bool type_is_function(const Type* type);
bool type_is_pointer(const Type* type);
bool type_is_numeric(const Type* type);
size_t type_get_vector_dimension(const Type* type);
bool type_get_matrix_dimensions(const Type* type, size_t* rows, size_t* cols);
Vector* type_get_tensor_dimensions(const Type* type);
bool type_get_function_signature(const Type* type, Vector** params, Type** return_type);
Type* type_get_pointer_type(const Type* type);
uint32_t* type_get_priority(const Type* type);
bool type_set_priority(Type* type, uint32_t priority);
bool type_is_compatible_with(const Type* type1, const Type* type2);
bool type_can_own(const Type* type1, const Type* type2);
char* type_to_string(const Type* type);

#endif // SLANG_TYPE_SYSTEM_H 