#include "type_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Type* type_new(TypeKind kind) {
    Type* type = (Type*)malloc(sizeof(Type));
    if (type == NULL) {
        return NULL;
    }
    
    type->kind = kind;
    return type;
}

void type_free(Type* type) {
    if (type != NULL) {
        switch (type->kind) {
            case TYPE_ARRAY:
                type_free(type->array.element_type);
                break;
            case TYPE_TUPLE:
                for (size_t i = 0; i < type->tuple.types->size; i++) {
                    Type* t = (Type*)((char*)type->tuple.types->data + i * sizeof(Type));
                    type_free(t);
                }
                vector_free(type->tuple.types);
                break;
            case TYPE_VECTOR:
                type_free(type->vector.element_type);
                break;
            case TYPE_MATRIX:
                type_free(type->matrix.element_type);
                break;
            case TYPE_TENSOR:
                vector_free(type->tensor.dimensions);
                type_free(type->tensor.element_type);
                break;
            case TYPE_QUATERNION:
                type_free(type->quaternion.element_type);
                break;
            case TYPE_COMPLEX:
                type_free(type->complex.element_type);
                break;
            case TYPE_FUNCTION:
                for (size_t i = 0; i < type->function.params->size; i++) {
                    Type* t = (Type*)((char*)type->function.params->data + i * sizeof(Type));
                    type_free(t);
                }
                vector_free(type->function.params);
                type_free(type->function.return_type);
                if (type->function.priority != NULL) {
                    free(type->function.priority);
                }
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
}

bool type_is_vector(const Type* type) {
    return type->kind == TYPE_VECTOR;
}

bool type_is_matrix(const Type* type) {
    return type->kind == TYPE_MATRIX;
}

bool type_is_tensor(const Type* type) {
    return type->kind == TYPE_TENSOR;
}

bool type_is_quaternion(const Type* type) {
    return type->kind == TYPE_QUATERNION;
}

bool type_is_complex(const Type* type) {
    return type->kind == TYPE_COMPLEX;
}

bool type_is_function(const Type* type) {
    return type->kind == TYPE_FUNCTION;
}

bool type_is_pointer(const Type* type) {
    return type->kind == TYPE_POINTER;
}

bool type_is_numeric(const Type* type) {
    return type->kind == TYPE_INT || type->kind == TYPE_FLOAT;
}

size_t* type_get_vector_dimension(const Type* type) {
    if (type->kind == TYPE_VECTOR) {
        return &type->vector.dimension;
    }
    return NULL;
}

bool type_get_matrix_dimensions(const Type* type, size_t* rows, size_t* cols) {
    if (type->kind == TYPE_MATRIX) {
        *rows = type->matrix.rows;
        *cols = type->matrix.columns;
        return true;
    }
    return false;
}

Vector* type_get_tensor_dimensions(const Type* type) {
    if (type->kind == TYPE_TENSOR) {
        return type->tensor.dimensions;
    }
    return NULL;
}

bool type_get_function_signature(const Type* type, Vector** params, Type** return_type) {
    if (type->kind == TYPE_FUNCTION) {
        *params = type->function.params;
        *return_type = type->function.return_type;
        return true;
    }
    return false;
}

Type* type_get_pointer_type(const Type* type) {
    if (type->kind == TYPE_POINTER) {
        return type->pointer.inner_type;
    }
    return NULL;
}

uint32_t* type_get_priority(const Type* type) {
    if (type->kind == TYPE_FUNCTION) {
        return type->function.priority;
    }
    return NULL;
}

bool type_set_priority(Type* type, uint32_t priority) {
    if (type->kind == TYPE_FUNCTION) {
        if (type->function.priority == NULL) {
            type->function.priority = (uint32_t*)malloc(sizeof(uint32_t));
            if (type->function.priority == NULL) {
                return false;
            }
        }
        *type->function.priority = priority;
        return true;
    }
    return false;
}

bool type_is_compatible_with(const Type* type1, const Type* type2) {
    if (type1->kind == type2->kind) {
        switch (type1->kind) {
            case TYPE_INT:
            case TYPE_FLOAT:
            case TYPE_BOOL:
            case TYPE_STRING:
            case TYPE_CHAR:
            case TYPE_VOID:
            case TYPE_UNIT:
                return true;
            case TYPE_ARRAY:
                return type_is_compatible_with(type1->array.element_type, type2->array.element_type);
            case TYPE_TUPLE:
                if (type1->tuple.types->size != type2->tuple.types->size) {
                    return false;
                }
                for (size_t i = 0; i < type1->tuple.types->size; i++) {
                    Type* t1 = (Type*)((char*)type1->tuple.types->data + i * sizeof(Type));
                    Type* t2 = (Type*)((char*)type2->tuple.types->data + i * sizeof(Type));
                    if (!type_is_compatible_with(t1, t2)) {
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
                if (type1->tensor.dimensions->size != type2->tensor.dimensions->size) {
                    return false;
                }
                for (size_t i = 0; i < type1->tensor.dimensions->size; i++) {
                    size_t* d1 = (size_t*)((char*)type1->tensor.dimensions->data + i * sizeof(size_t));
                    size_t* d2 = (size_t*)((char*)type2->tensor.dimensions->data + i * sizeof(size_t));
                    if (*d1 != *d2) {
                        return false;
                    }
                }
                return type_is_compatible_with(type1->tensor.element_type, type2->tensor.element_type);
            case TYPE_QUATERNION:
                return type_is_compatible_with(type1->quaternion.element_type, type2->quaternion.element_type);
            case TYPE_COMPLEX:
                return type_is_compatible_with(type1->complex.element_type, type2->complex.element_type);
            case TYPE_FUNCTION:
                if (type1->function.params->size != type2->function.params->size) {
                    return false;
                }
                for (size_t i = 0; i < type1->function.params->size; i++) {
                    Type* p1 = (Type*)((char*)type1->function.params->data + i * sizeof(Type));
                    Type* p2 = (Type*)((char*)type2->function.params->data + i * sizeof(Type));
                    if (!type_is_compatible_with(p1, p2)) {
                        return false;
                    }
                }
                return type_is_compatible_with(type1->function.return_type, type2->function.return_type);
            case TYPE_POINTER:
                return type_is_compatible_with(type1->pointer.inner_type, type2->pointer.inner_type);
            case TYPE_NAMED:
                return strcmp(type1->named.name, type2->named.name) == 0;
        }
    }
    
    // Special cases for type compatibility
    if ((type1->kind == TYPE_INT && type2->kind == TYPE_FLOAT) ||
        (type1->kind == TYPE_FLOAT && type2->kind == TYPE_INT)) {
        return true;
    }
    
    if ((type1->kind == TYPE_INT || type1->kind == TYPE_FLOAT || type1->kind == TYPE_BOOL) &&
        type2->kind == TYPE_STRING) {
        return true;
    }
    
    if (type1->kind == TYPE_STRING &&
        (type2->kind == TYPE_INT || type2->kind == TYPE_FLOAT || type2->kind == TYPE_BOOL)) {
        return true;
    }
    
    return false;
}

bool type_can_own(const Type* type1, const Type* type2) {
    if (type1->kind == TYPE_FUNCTION && type2->kind == TYPE_FUNCTION) {
        uint32_t* p1 = type_get_priority(type1);
        uint32_t* p2 = type_get_priority(type2);
        return p1 != NULL && p2 != NULL && *p1 > *p2;
    }
    return false;
}

char* type_to_string(const Type* type) {
    char buffer[1024];
    char* result;
    
    switch (type->kind) {
        case TYPE_UNIT:
            strcpy(buffer, "()");
            break;
        case TYPE_INT:
            strcpy(buffer, "int");
            break;
        case TYPE_FLOAT:
            strcpy(buffer, "float");
            break;
        case TYPE_BOOL:
            strcpy(buffer, "bool");
            break;
        case TYPE_STRING:
            strcpy(buffer, "string");
            break;
        case TYPE_CHAR:
            strcpy(buffer, "char");
            break;
        case TYPE_VOID:
            strcpy(buffer, "void");
            break;
        case TYPE_ARRAY: {
            char* element_str = type_to_string(type->array.element_type);
            snprintf(buffer, sizeof(buffer), "[%s]", element_str);
            free(element_str);
            break;
        }
        case TYPE_TUPLE: {
            char* tuple_str = (char*)malloc(1024);
            strcpy(tuple_str, "(");
            for (size_t i = 0; i < type->tuple.types->size; i++) {
                Type* t = (Type*)((char*)type->tuple.types->data + i * sizeof(Type));
                char* type_str = type_to_string(t);
                if (i > 0) {
                    strcat(tuple_str, ", ");
                }
                strcat(tuple_str, type_str);
                free(type_str);
            }
            strcat(tuple_str, ")");
            strcpy(buffer, tuple_str);
            free(tuple_str);
            break;
        }
        case TYPE_VECTOR: {
            char* element_str = type_to_string(type->vector.element_type);
            snprintf(buffer, sizeof(buffer), "vec%zu<%s>", type->vector.dimension, element_str);
            free(element_str);
            break;
        }
        case TYPE_MATRIX: {
            char* element_str = type_to_string(type->matrix.element_type);
            snprintf(buffer, sizeof(buffer), "mat%zux%zu<%s>",
                    type->matrix.rows, type->matrix.columns, element_str);
            free(element_str);
            break;
        }
        case TYPE_TENSOR: {
            char* dims_str = (char*)malloc(1024);
            strcpy(dims_str, "");
            for (size_t i = 0; i < type->tensor.dimensions->size; i++) {
                size_t* dim = (size_t*)((char*)type->tensor.dimensions->data + i * sizeof(size_t));
                if (i > 0) {
                    strcat(dims_str, "x");
                }
                char dim_str[32];
                snprintf(dim_str, sizeof(dim_str), "%zu", *dim);
                strcat(dims_str, dim_str);
            }
            char* element_str = type_to_string(type->tensor.element_type);
            snprintf(buffer, sizeof(buffer), "tensor<%s, %s>", dims_str, element_str);
            free(dims_str);
            free(element_str);
            break;
        }
        case TYPE_QUATERNION: {
            char* element_str = type_to_string(type->quaternion.element_type);
            snprintf(buffer, sizeof(buffer), "quat<%s>", element_str);
            free(element_str);
            break;
        }
        case TYPE_COMPLEX: {
            char* element_str = type_to_string(type->complex.element_type);
            snprintf(buffer, sizeof(buffer), "complex<%s>", element_str);
            free(element_str);
            break;
        }
        case TYPE_FUNCTION: {
            char* params_str = (char*)malloc(1024);
            strcpy(params_str, "fn(");
            for (size_t i = 0; i < type->function.params->size; i++) {
                Type* t = (Type*)((char*)type->function.params->data + i * sizeof(Type));
                char* type_str = type_to_string(t);
                if (i > 0) {
                    strcat(params_str, ", ");
                }
                strcat(params_str, type_str);
                free(type_str);
            }
            strcat(params_str, ") -> ");
            char* return_str = type_to_string(type->function.return_type);
            strcat(params_str, return_str);
            free(return_str);
            
            if (type->function.priority != NULL) {
                char priority_str[32];
                snprintf(priority_str, sizeof(priority_str), " @%u", *type->function.priority);
                strcat(params_str, priority_str);
            }
            
            strcpy(buffer, params_str);
            free(params_str);
            break;
        }
        case TYPE_POINTER: {
            char* inner_str = type_to_string(type->pointer.inner_type);
            snprintf(buffer, sizeof(buffer), "*%s", inner_str);
            free(inner_str);
            break;
        }
        case TYPE_NAMED:
            strcpy(buffer, type->named.name);
            break;
    }
    
    result = strdup(buffer);
    return result;
} 