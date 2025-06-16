#include "../include/type_system.h"
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
            type->array.size = 0;
            break;
        case TYPE_TUPLE:
            type->tuple.elements = NULL;
            type->tuple.size = 0;
            break;
        case TYPE_VECTOR:
            type->vector.dimensions = 0;
            type->vector.element_type = NULL;
            break;
        case TYPE_MATRIX:
            type->matrix.rows = 0;
            type->matrix.cols = 0;
            type->matrix.element_type = NULL;
            break;
        case TYPE_TENSOR:
            type->tensor.rank = 0;
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
            type->function.parameters = NULL;
            type->function.return_type = NULL;
            type->function.parameter_count = 0;
            break;
        case TYPE_POINTER:
            type->pointer.pointed_type = NULL;
            break;
        case TYPE_NAMED:
            type->named.name = NULL;
            type->named.underlying_type = NULL;
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
            for (size_t i = 0; i < type->tuple.size; i++) {
                type_free(type->tuple.elements[i]);
            }
            free(type->tuple.elements);
            break;
        case TYPE_VECTOR:
            type_free(type->vector.element_type);
            break;
        case TYPE_MATRIX:
            type_free(type->matrix.element_type);
            break;
        case TYPE_TENSOR:
            type_free(type->tensor.element_type);
            free(type->tensor.dimensions);
            break;
        case TYPE_QUATERNION:
            type_free(type->quaternion.element_type);
            break;
        case TYPE_COMPLEX:
            type_free(type->complex.element_type);
            break;
        case TYPE_FUNCTION:
            for (size_t i = 0; i < type->function.parameter_count; i++) {
                type_free(type->function.parameters[i]);
            }
            free(type->function.parameters);
            type_free(type->function.return_type);
            break;
        case TYPE_POINTER:
            type_free(type->pointer.pointed_type);
            break;
        case TYPE_NAMED:
            free(type->named.name);
            type_free(type->named.underlying_type);
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
size_t type_get_vector_dimensions(const Type* type) {
    return type_is_vector(type) ? type->vector.dimensions : 0;
}

size_t type_get_matrix_rows(const Type* type) {
    return type_is_matrix(type) ? type->matrix.rows : 0;
}

size_t type_get_matrix_cols(const Type* type) {
    return type_is_matrix(type) ? type->matrix.cols : 0;
}

size_t type_get_tensor_rank(const Type* type) {
    return type_is_tensor(type) ? type->tensor.rank : 0;
}

const size_t* type_get_tensor_dimensions(const Type* type) {
    return type_is_tensor(type) ? type->tensor.dimensions : NULL;
}

const Type* type_get_element_type(const Type* type) {
    if (!type) return NULL;
    
    switch (type->kind) {
        case TYPE_ARRAY:
            return type->array.element_type;
        case TYPE_VECTOR:
            return type->vector.element_type;
        case TYPE_MATRIX:
            return type->matrix.element_type;
        case TYPE_TENSOR:
            return type->tensor.element_type;
        case TYPE_QUATERNION:
            return type->quaternion.element_type;
        case TYPE_COMPLEX:
            return type->complex.element_type;
        case TYPE_POINTER:
            return type->pointer.pointed_type;
        default:
            return NULL;
    }
}

const Type* type_get_function_return_type(const Type* type) {
    return type_is_function(type) ? type->function.return_type : NULL;
}

size_t type_get_function_parameter_count(const Type* type) {
    return type_is_function(type) ? type->function.parameter_count : 0;
}

const Type* type_get_function_parameter_type(const Type* type, size_t index) {
    if (!type_is_function(type) || index >= type->function.parameter_count) {
        return NULL;
    }
    return type->function.parameters[index];
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
        case TYPE_NULL:
            return true;
            
        case TYPE_ARRAY:
            return type_is_compatible_with(type1->array.element_type, type2->array.element_type) &&
                   type1->array.size == type2->array.size;
            
        case TYPE_TUPLE:
            if (type1->tuple.size != type2->tuple.size) return false;
            for (size_t i = 0; i < type1->tuple.size; i++) {
                if (!type_is_compatible_with(type1->tuple.elements[i], type2->tuple.elements[i])) {
                    return false;
                }
            }
            return true;
            
        case TYPE_VECTOR:
            return type1->vector.dimensions == type2->vector.dimensions &&
                   type_is_compatible_with(type1->vector.element_type, type2->vector.element_type);
            
        case TYPE_MATRIX:
            return type1->matrix.rows == type2->matrix.rows &&
                   type1->matrix.cols == type2->matrix.cols &&
                   type_is_compatible_with(type1->matrix.element_type, type2->matrix.element_type);
            
        case TYPE_TENSOR:
            if (type1->tensor.rank != type2->tensor.rank) return false;
            for (size_t i = 0; i < type1->tensor.rank; i++) {
                if (type1->tensor.dimensions[i] != type2->tensor.dimensions[i]) {
                    return false;
                }
            }
            return type_is_compatible_with(type1->tensor.element_type, type2->tensor.element_type);
            
        case TYPE_QUATERNION:
            return type_is_compatible_with(type1->quaternion.element_type, type2->quaternion.element_type);
            
        case TYPE_COMPLEX:
            return type_is_compatible_with(type1->complex.element_type, type2->complex.element_type);
            
        case TYPE_FUNCTION:
            if (type1->function.parameter_count != type2->function.parameter_count) return false;
            if (!type_is_compatible_with(type1->function.return_type, type2->function.return_type)) {
                return false;
            }
            for (size_t i = 0; i < type1->function.parameter_count; i++) {
                if (!type_is_compatible_with(type1->function.parameters[i], type2->function.parameters[i])) {
                    return false;
                }
            }
            return true;
            
        case TYPE_POINTER:
            return type_is_compatible_with(type1->pointer.pointed_type, type2->pointer.pointed_type);
            
        case TYPE_NAMED:
            return strcmp(type1->named.name, type2->named.name) == 0 &&
                   type_is_compatible_with(type1->named.underlying_type, type2->named.underlying_type);
            
        default:
            return false;
    }
}

bool type_can_own(const Type* type1, const Type* type2) {
    if (!type1 || !type2) return false;
    
    if (type1->kind != TYPE_FUNCTION || type2->kind != TYPE_FUNCTION) {
        return false;
    }
    
    if (type1->function.parameter_count != type2->function.parameter_count) {
        return false;
    }
    
    for (size_t i = 0; i < type1->function.parameter_count; i++) {
        const Type* param1 = type1->function.parameters[i];
        const Type* param2 = type2->function.parameters[i];
        
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
            
        case TYPE_NULL:
            result = strdup("null");
            break;
            
        case TYPE_ARRAY: {
            char* element_str = type_to_string(type->array.element_type);
            asprintf(&result, "[%s; %zu]", element_str, type->array.size);
            free(element_str);
            break;
        }
            
        case TYPE_TUPLE: {
            char* elements[type->tuple.size];
            size_t total_len = 2; // For "()"
            
            for (size_t i = 0; i < type->tuple.size; i++) {
                elements[i] = type_to_string(type->tuple.elements[i]);
                total_len += strlen(elements[i]) + 2; // +2 for ", "
            }
            
            result = malloc(total_len);
            char* ptr = result;
            *ptr++ = '(';
            
            for (size_t i = 0; i < type->tuple.size; i++) {
                strcpy(ptr, elements[i]);
                ptr += strlen(elements[i]);
                if (i < type->tuple.size - 1) {
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
            asprintf(&result, "vec%zu<%s>", type->vector.dimensions, element_str);
            free(element_str);
            break;
        }
            
        case TYPE_MATRIX: {
            char* element_str = type_to_string(type->matrix.element_type);
            asprintf(&result, "mat%zux%zu<%s>", type->matrix.rows, type->matrix.cols, element_str);
            free(element_str);
            break;
        }
            
        case TYPE_TENSOR: {
            char* element_str = type_to_string(type->tensor.element_type);
            char* dims = malloc(type->tensor.rank * 4 + 1); // Each dimension can be up to 3 digits + 'x'
            char* ptr = dims;
            
            for (size_t i = 0; i < type->tensor.rank; i++) {
                ptr += sprintf(ptr, "%zux", type->tensor.dimensions[i]);
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
            char* params[type->function.parameter_count];
            size_t total_len = 2; // For "()"
            
            for (size_t i = 0; i < type->function.parameter_count; i++) {
                params[i] = type_to_string(type->function.parameters[i]);
                total_len += strlen(params[i]) + 2; // +2 for ", "
            }
            
            char* return_str = type_to_string(type->function.return_type);
            total_len += strlen(return_str) + 4; // +4 for " -> "
            
            result = malloc(total_len);
            char* ptr = result;
            *ptr++ = '(';
            
            for (size_t i = 0; i < type->function.parameter_count; i++) {
                strcpy(ptr, params[i]);
                ptr += strlen(params[i]);
                if (i < type->function.parameter_count - 1) {
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
            char* pointed_str = type_to_string(type->pointer.pointed_type);
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