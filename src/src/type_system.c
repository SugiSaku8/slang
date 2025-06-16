#include "../include/type_system.h"
#include "../include/ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Type creation and destruction
Type* type_new(TypeKind kind) {
    Type* type = (Type*)malloc(sizeof(Type));
    if (!type) return NULL;
    type->kind = kind;
    switch (kind) {
        case TYPE_ARRAY:
            type->array.element_type = NULL;
            break;
        case TYPE_TUPLE:
            type->tuple.types = NULL;
            break;
        case TYPE_VECTOR:
            type->vector.dimension = 0;
            type->vector.element_type = NULL;
            break;
        case TYPE_MATRIX:
            type->matrix.rows = 0;
            type->matrix.columns = 0;
            type->matrix.element_type = NULL;
            break;
        case TYPE_TENSOR:
            type->tensor.dimensions = NULL;
            type->tensor.element_type = NULL;
            break;
        case TYPE_QUATERNION:
            type->quaternion.element_type = NULL;
            break;
        case TYPE_COMPLEX:
            type->complex.element_type = NULL;
            break;
        case TYPE_FUNCTION:
            type->function.params = NULL;
            type->function.return_type = NULL;
            type->function.priority = NULL;
            break;
        case TYPE_POINTER:
            type->pointer.inner_type = NULL;
            break;
        case TYPE_NAMED:
            type->named.name = NULL;
            break;
        default:
            break;
    }
    return type;
}

void type_free(Type* type) {
    if (!type) return;
    switch (type->kind) {
        case TYPE_ARRAY:
            type_free(type->array.element_type);
            break;
        case TYPE_TUPLE:
            if (type->tuple.types) {
                for (size_t i = 0; i < type->tuple.types->size; i++) {
                    type_free(((Type**)type->tuple.types->data)[i]);
                }
                vector_free(type->tuple.types);
            }
            break;
        case TYPE_VECTOR:
            type_free(type->vector.element_type);
            break;
        case TYPE_MATRIX:
            type_free(type->matrix.element_type);
            break;
        case TYPE_TENSOR:
            if (type->tensor.dimensions) {
                free(type->tensor.dimensions->data);
                vector_free(type->tensor.dimensions);
            }
            type_free(type->tensor.element_type);
            break;
        case TYPE_QUATERNION:
            type_free(type->quaternion.element_type);
            break;
        case TYPE_COMPLEX:
            type_free(type->complex.element_type);
            break;
        case TYPE_FUNCTION:
            if (type->function.params) {
                for (size_t i = 0; i < type->function.params->size; i++) {
                    type_free(((Type**)type->function.params->data)[i]);
                }
                vector_free(type->function.params);
            }
            type_free(type->function.return_type);
            if (type->function.priority) free(type->function.priority);
            break;
        case TYPE_POINTER:
            type_free(type->pointer.inner_type);
            break;
        case TYPE_NAMED:
            free(type->named.name);
            break;
        default:
            break;
    }
    free(type);
}

// Type kind checks
bool type_is_vector(const Type* type) {
    return type && type->kind == TYPE_VECTOR;
}

bool type_is_matrix(const Type* type) {
    return type && type->kind == TYPE_MATRIX;
}

bool type_is_tensor(const Type* type) {
    return type && type->kind == TYPE_TENSOR;
}

bool type_is_quaternion(const Type* type) {
    return type && type->kind == TYPE_QUATERNION;
}

bool type_is_complex(const Type* type) {
    return type && type->kind == TYPE_COMPLEX;
}

bool type_is_function(const Type* type) {
    return type && type->kind == TYPE_FUNCTION;
}

bool type_is_pointer(const Type* type) {
    return type && type->kind == TYPE_POINTER;
}

bool type_is_named(const Type* type) {
    return type && type->kind == TYPE_NAMED;
}

// Type information getters
size_t type_get_vector_dimension(const Type* type) {
    return type_is_vector(type) ? type->vector.dimension : 0;
}

bool type_get_matrix_dimensions(const Type* type, size_t* rows, size_t* cols) {
    if (type_is_matrix(type)) {
        *rows = type->matrix.rows;
        *cols = type->matrix.columns;
        return true;
    }
    return false;
}

Vector* type_get_tensor_dimensions(const Type* type) {
    return type_is_tensor(type) ? type->tensor.dimensions : NULL;
}

bool type_get_function_signature(const Type* type, Vector** params, Type** return_type) {
    if (type_is_function(type)) {
        *params = type->function.params;
        *return_type = type->function.return_type;
        return true;
    }
    return false;
}

Type* type_get_pointer_type(const Type* type) {
    return type_is_pointer(type) ? type->pointer.inner_type : NULL;
}

uint32_t* type_get_priority(const Type* type) {
    return type_is_function(type) ? type->function.priority : NULL;
}

bool type_set_priority(Type* type, uint32_t priority) {
    if (type_is_function(type)) {
        if (!type->function.priority) {
            type->function.priority = (uint32_t*)malloc(sizeof(uint32_t));
            if (!type->function.priority) return false;
        }
        *type->function.priority = priority;
        return true;
    }
    return false;
}

// Type compatibility
bool type_is_compatible_with(const Type* type1, const Type* type2) {
    if (!type1 || !type2) return false;
    
    if (type1->kind != type2->kind) return false;
    
    switch (type1->kind) {
        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_BOOL:
        case TYPE_STRING:
            return true;
            
        case TYPE_ARRAY:
            return type_is_compatible_with(type1->array.element_type, type2->array.element_type);
            
        case TYPE_TUPLE:
            if (type1->tuple.types->size != type2->tuple.types->size) return false;
            for (size_t i = 0; i < type1->tuple.types->size; i++) {
                if (!type_is_compatible_with(((Type**)type1->tuple.types->data)[i], ((Type**)type2->tuple.types->data)[i])) {
                    return false;
                }
            }
            return true;
            
        case TYPE_VECTOR:
            return type1->vector.dimension == type2->vector.dimension &&
                   type_is_compatible_with(type1->vector.element_type, type2->vector.element_type);
            
        case TYPE_MATRIX:
            return type1->matrix.rows == type2->matrix.rows &&
                   type1->matrix.columns == type2->matrix.columns &&
                   type_is_compatible_with(type1->matrix.element_type, type2->matrix.element_type);
            
        case TYPE_TENSOR:
            if (type1->tensor.dimensions->size != type2->tensor.dimensions->size) return false;
            for (size_t i = 0; i < type1->tensor.dimensions->size; i++) {
                if (((size_t*)type1->tensor.dimensions->data)[i] != ((size_t*)type2->tensor.dimensions->data)[i]) {
                    return false;
                }
            }
            return type_is_compatible_with(type1->tensor.element_type, type2->tensor.element_type);
            
        case TYPE_QUATERNION:
            return type_is_compatible_with(type1->quaternion.element_type, type2->quaternion.element_type);
            
        case TYPE_COMPLEX:
            return type_is_compatible_with(type1->complex.element_type, type2->complex.element_type);
            
        case TYPE_FUNCTION:
            if (type1->function.params->size != type2->function.params->size) return false;
            if (!type_is_compatible_with(type1->function.return_type, type2->function.return_type)) {
                return false;
            }
            for (size_t i = 0; i < type1->function.params->size; i++) {
                if (!type_is_compatible_with(((Type**)type1->function.params->data)[i], ((Type**)type2->function.params->data)[i])) {
                    return false;
                }
            }
            return true;
            
        case TYPE_POINTER:
            return type_is_compatible_with(type1->pointer.inner_type, type2->pointer.inner_type);
            
        case TYPE_NAMED:
            return strcmp(type1->named.name, type2->named.name) == 0;
            
        default:
            return false;
    }
}

bool type_can_own(const Type* type1, const Type* type2) {
    if (!type1 || !type2) return false;
    
    if (type1->kind != TYPE_FUNCTION || type2->kind != TYPE_FUNCTION) {
        return false;
    }
    
    if (type1->function.params->size != type2->function.params->size) {
        return false;
    }
    
    for (size_t i = 0; i < type1->function.params->size; i++) {
        const Type* param1 = ((Type**)type1->function.params->data)[i];
        const Type* param2 = ((Type**)type2->function.params->data)[i];
        
        if (!type_is_compatible_with(param1, param2)) {
            return false;
        }
    }
    
    return type_is_compatible_with(type1->function.return_type, type2->function.return_type);
}

// Type to string conversion
char* type_to_string(const Type* type) {
    if (!type) return strdup("unknown");
    
    char* result = NULL;
    switch (type->kind) {
        case TYPE_INT:
            result = strdup("int");
            break;
            
        case TYPE_FLOAT:
            result = strdup("float");
            break;
            
        case TYPE_BOOL:
            result = strdup("bool");
            break;
            
        case TYPE_STRING:
            result = strdup("string");
            break;
            
        case TYPE_ARRAY: {
            char* element_str = type_to_string(type->array.element_type);
            asprintf(&result, "[%s]", element_str);
            free(element_str);
            break;
        }
            
        case TYPE_TUPLE: {
            char* elements[type->tuple.types->size];
            size_t total_len = 2; // For "()"
            
            for (size_t i = 0; i < type->tuple.types->size; i++) {
                elements[i] = type_to_string(((Type**)type->tuple.types->data)[i]);
                total_len += strlen(elements[i]) + 2; // +2 for ", "
            }
            
            result = malloc(total_len);
            char* ptr = result;
            *ptr++ = '(';
            
            for (size_t i = 0; i < type->tuple.types->size; i++) {
                strcpy(ptr, elements[i]);
                ptr += strlen(elements[i]);
                if (i < type->tuple.types->size - 1) {
                    *ptr++ = ',';
                    *ptr++ = ' ';
                }
                free(elements[i]);
            }
            *ptr++ = ')';
            *ptr = '\0';
            break;
        }
            
        case TYPE_VECTOR: {
            char* element_str = type_to_string(type->vector.element_type);
            asprintf(&result, "vec%zu<%s>", type->vector.dimension, element_str);
            free(element_str);
            break;
        }
            
        case TYPE_MATRIX: {
            char* element_str = type_to_string(type->matrix.element_type);
            asprintf(&result, "mat%zux%zu<%s>", type->matrix.rows, type->matrix.columns, element_str);
            free(element_str);
            break;
        }
            
        case TYPE_TENSOR: {
            char* element_str = type_to_string(type->tensor.element_type);
            char* dims = malloc(type->tensor.dimensions->size * 4 + 1); // Each dimension can be up to 3 digits + 'x'
            char* ptr = dims;
            
            for (size_t i = 0; i < type->tensor.dimensions->size; i++) {
                ptr += sprintf(ptr, "%zux", ((size_t*)type->tensor.dimensions->data)[i]);
            }
            *(ptr - 1) = '\0'; // Remove last 'x'
            
            asprintf(&result, "tensor<%s><%s>", dims, element_str);
            free(dims);
            free(element_str);
            break;
        }
            
        case TYPE_QUATERNION: {
            char* element_str = type_to_string(type->quaternion.element_type);
            asprintf(&result, "quat<%s>", element_str);
            free(element_str);
            break;
        }
            
        case TYPE_COMPLEX: {
            char* element_str = type_to_string(type->complex.element_type);
            asprintf(&result, "complex<%s>", element_str);
            free(element_str);
            break;
        }
            
        case TYPE_FUNCTION: {
            char* params[type->function.params->size];
            size_t total_len = 2; // For "()"
            
            for (size_t i = 0; i < type->function.params->size; i++) {
                params[i] = type_to_string(((Type**)type->function.params->data)[i]);
                total_len += strlen(params[i]) + 2; // +2 for ", "
            }
            
            char* return_str = type_to_string(type->function.return_type);
            total_len += strlen(return_str) + 4; // +4 for " -> "
            
            result = malloc(total_len);
            char* ptr = result;
            *ptr++ = '(';
            
            for (size_t i = 0; i < type->function.params->size; i++) {
                strcpy(ptr, params[i]);
                ptr += strlen(params[i]);
                if (i < type->function.params->size - 1) {
                    *ptr++ = ',';
                    *ptr++ = ' ';
                }
                free(params[i]);
            }
            
            strcpy(ptr, ") -> ");
            ptr += 6;
            strcpy(ptr, return_str);
            free(return_str);
            break;
        }
            
        case TYPE_POINTER: {
            char* pointed_str = type_to_string(type->pointer.inner_type);
            asprintf(&result, "*%s", pointed_str);
            free(pointed_str);
            break;
        }
            
        case TYPE_NAMED:
            result = strdup(type->named.name);
            break;
            
        default:
            result = strdup("unknown");
            break;
    }
    
    return result;
} 