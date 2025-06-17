#include "../include/type_system.h"
#include "../include/ast.h"
#include "../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Type creation and destruction
Type* type_new(TypeKind kind) {
    Type* type = (Type*)malloc(sizeof(Type));
    if (!type) return NULL;
    type->kind = kind;
    type->size = 0;
    type->is_mutable = false;
    switch (kind) {
        case TYPE_ARRAY:
            type->data.array.element_type = NULL;
            break;
        case TYPE_TUPLE:
            type->data.tuple.types = NULL;
            type->data.tuple.type_count = 0;
            break;
        case TYPE_VECTOR:
            type->data.vector.dimension = 0;
            type->data.vector.element_type = NULL;
            break;
        case TYPE_MATRIX:
            type->data.matrix.rows = 0;
            type->data.matrix.columns = 0;
            type->data.matrix.element_type = NULL;
            break;
        case TYPE_TENSOR:
            type->data.tensor.dimensions = NULL;
            type->data.tensor.dimension_count = 0;
            type->data.tensor.element_type = NULL;
            break;
        case TYPE_QUATERNION:
            type->data.quaternion.element_type = NULL;
            break;
        case TYPE_COMPLEX:
            type->data.complex.element_type = NULL;
            break;
        case TYPE_FUNCTION:
            type->data.function.parameter_types = NULL;
            type->data.function.parameter_count = 0;
            type->data.function.return_type = NULL;
            break;
        case TYPE_NAMED:
            type->data.named.name = NULL;
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
            type_free(type->data.array.element_type);
            break;
        case TYPE_TUPLE:
            if (type->data.tuple.types) {
                for (size_t i = 0; i < type->data.tuple.type_count; i++) {
                    type_free(type->data.tuple.types[i]);
                }
                free(type->data.tuple.types);
            }
            break;
        case TYPE_VECTOR:
            type_free(type->data.vector.element_type);
            break;
        case TYPE_MATRIX:
            type_free(type->data.matrix.element_type);
            break;
        case TYPE_TENSOR:
            if (type->data.tensor.dimensions) {
                free(type->data.tensor.dimensions);
            }
            type_free(type->data.tensor.element_type);
            break;
        case TYPE_QUATERNION:
            type_free(type->data.quaternion.element_type);
            break;
        case TYPE_COMPLEX:
            type_free(type->data.complex.element_type);
            break;
        case TYPE_FUNCTION:
            if (type->data.function.parameter_types) {
                for (size_t i = 0; i < type->data.function.parameter_count; i++) {
                    type_free(type->data.function.parameter_types[i]);
                }
                free(type->data.function.parameter_types);
            }
            type_free(type->data.function.return_type);
            break;
        case TYPE_NAMED:
            free(type->data.named.name);
            break;
        default:
            break;
    }
    free(type);
}

void free_type(Type* type) { type_free(type); } // wrapper for compatibility

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

bool type_is_named(const Type* type) {
    return type && type->kind == TYPE_NAMED;
}

// Type information getters
size_t type_get_vector_dimension(const Type* type) {
    return type_is_vector(type) ? type->data.vector.dimension : 0;
}

bool type_get_matrix_dimensions(const Type* type, size_t* rows, size_t* cols) {
    if (type_is_matrix(type)) {
        *rows = type->data.matrix.rows;
        *cols = type->data.matrix.columns;
        return true;
    }
    return false;
}

size_t* type_get_tensor_dimensions(const Type* type, size_t* count) {
    if (type_is_tensor(type)) {
        *count = type->data.tensor.dimension_count;
        return type->data.tensor.dimensions;
    }
    *count = 0;
    return NULL;
}

bool type_get_function_signature(const Type* type, Type*** params, size_t* param_count, Type** return_type) {
    if (type_is_function(type)) {
        *params = type->data.function.parameter_types;
        *param_count = type->data.function.parameter_count;
        *return_type = type->data.function.return_type;
        return true;
    }
    return false;
}

// Type compatibility
bool type_is_compatible_with(const Type* type1, const Type* type2) {
    if (!type1 || !type2) return false;
    
    if (type1->kind != type2->kind) return false;
    
    switch (type1->kind) {
        case TYPE_INTEGER:
        case TYPE_FLOAT:
        case TYPE_BOOLEAN:
        case TYPE_STRING:
            return true;
            
        case TYPE_ARRAY:
            return type_is_compatible_with(type1->data.array.element_type, type2->data.array.element_type);
            
        case TYPE_TUPLE:
            if (type1->data.tuple.type_count != type2->data.tuple.type_count) return false;
            for (size_t i = 0; i < type1->data.tuple.type_count; i++) {
                if (!type_is_compatible_with(type1->data.tuple.types[i], type2->data.tuple.types[i])) {
                    return false;
                }
            }
            return true;
            
        case TYPE_VECTOR:
            return type1->data.vector.dimension == type2->data.vector.dimension &&
                   type_is_compatible_with(type1->data.vector.element_type, type2->data.vector.element_type);
            
        case TYPE_MATRIX:
            return type1->data.matrix.rows == type2->data.matrix.rows &&
                   type1->data.matrix.columns == type2->data.matrix.columns &&
                   type_is_compatible_with(type1->data.matrix.element_type, type2->data.matrix.element_type);
            
        case TYPE_TENSOR:
            if (type1->data.tensor.dimension_count != type2->data.tensor.dimension_count) return false;
            for (size_t i = 0; i < type1->data.tensor.dimension_count; i++) {
                if (type1->data.tensor.dimensions[i] != type2->data.tensor.dimensions[i]) {
                    return false;
                }
            }
            return type_is_compatible_with(type1->data.tensor.element_type, type2->data.tensor.element_type);
            
        case TYPE_QUATERNION:
            return type_is_compatible_with(type1->data.quaternion.element_type, type2->data.quaternion.element_type);
            
        case TYPE_COMPLEX:
            return type_is_compatible_with(type1->data.complex.element_type, type2->data.complex.element_type);
            
        case TYPE_FUNCTION:
            if (type1->data.function.parameter_count != type2->data.function.parameter_count) return false;
            if (!type_is_compatible_with(type1->data.function.return_type, type2->data.function.return_type)) {
                return false;
            }
            for (size_t i = 0; i < type1->data.function.parameter_count; i++) {
                if (!type_is_compatible_with(type1->data.function.parameter_types[i], type2->data.function.parameter_types[i])) {
                    return false;
                }
            }
            return true;
            
        case TYPE_NAMED:
            return strcmp(type1->data.named.name, type2->data.named.name) == 0;
            
        default:
            return false;
    }
}

bool type_can_own(const Type* type1, const Type* type2) {
    if (!type1 || !type2) return false;
    
    if (type1->kind != TYPE_FUNCTION || type2->kind != TYPE_FUNCTION) {
        return false;
    }
    
    if (type1->data.function.parameter_count != type2->data.function.parameter_count) {
        return false;
    }
    
    for (size_t i = 0; i < type1->data.function.parameter_count; i++) {
        const Type* param1 = type1->data.function.parameter_types[i];
        const Type* param2 = type2->data.function.parameter_types[i];
        
        if (!type_is_compatible_with(param1, param2)) {
            return false;
        }
    }
    
    return type_is_compatible_with(type1->data.function.return_type, type2->data.function.return_type);
}

// Type to string conversion
char* type_to_string(const Type* type) {
    if (!type) return strdup("unknown");
    
    char* result = NULL;
    switch (type->kind) {
        case TYPE_INTEGER:
            result = strdup("int");
            break;
            
        case TYPE_FLOAT:
            result = strdup("float");
            break;
            
        case TYPE_BOOLEAN:
            result = strdup("bool");
            break;
            
        case TYPE_STRING:
            result = strdup("string");
            break;
            
        case TYPE_ARRAY: {
            char* element_str = type_to_string(type->data.array.element_type);
            asprintf(&result, "[%s]", element_str);
            free(element_str);
            break;
        }
            
        case TYPE_TUPLE: {
            char* elements[type->data.tuple.type_count];
            size_t total_len = 2; // For "()"
            
            for (size_t i = 0; i < type->data.tuple.type_count; i++) {
                elements[i] = type_to_string(type->data.tuple.types[i]);
                total_len += strlen(elements[i]) + 2; // +2 for ", "
            }
            
            result = malloc(total_len);
            char* ptr = result;
            *ptr++ = '(';
            
            for (size_t i = 0; i < type->data.tuple.type_count; i++) {
                strcpy(ptr, elements[i]);
                ptr += strlen(elements[i]);
                if (i < type->data.tuple.type_count - 1) {
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
            char* element_str = type_to_string(type->data.vector.element_type);
            asprintf(&result, "vec%zu<%s>", type->data.vector.dimension, element_str);
            free(element_str);
            break;
        }
            
        case TYPE_MATRIX: {
            char* element_str = type_to_string(type->data.matrix.element_type);
            asprintf(&result, "mat%zux%zu<%s>", type->data.matrix.rows, type->data.matrix.columns, element_str);
            free(element_str);
            break;
        }
            
        case TYPE_TENSOR: {
            char* element_str = type_to_string(type->data.tensor.element_type);
            char* dims = malloc(type->data.tensor.dimension_count * 4 + 1); // Each dimension can be up to 3 digits + 'x'
            char* ptr = dims;
            
            for (size_t i = 0; i < type->data.tensor.dimension_count; i++) {
                ptr += sprintf(ptr, "%zux", type->data.tensor.dimensions[i]);
            }
            *(ptr - 1) = '\0'; // Remove last 'x'
            
            asprintf(&result, "tensor<%s><%s>", dims, element_str);
            free(dims);
            free(element_str);
            break;
        }
            
        case TYPE_QUATERNION: {
            char* element_str = type_to_string(type->data.quaternion.element_type);
            asprintf(&result, "quat<%s>", element_str);
            free(element_str);
            break;
        }
            
        case TYPE_COMPLEX: {
            char* element_str = type_to_string(type->data.complex.element_type);
            asprintf(&result, "complex<%s>", element_str);
            free(element_str);
            break;
        }
            
        case TYPE_FUNCTION: {
            char* params[type->data.function.parameter_count];
            size_t total_len = 2; // For "()"
            
            for (size_t i = 0; i < type->data.function.parameter_count; i++) {
                params[i] = type_to_string(type->data.function.parameter_types[i]);
                total_len += strlen(params[i]) + 2; // +2 for ", "
            }
            
            char* return_str = type_to_string(type->data.function.return_type);
            total_len += strlen(return_str) + 4; // +4 for " -> "
            
            result = malloc(total_len);
            char* ptr = result;
            *ptr++ = '(';
            
            for (size_t i = 0; i < type->data.function.parameter_count; i++) {
                strcpy(ptr, params[i]);
                ptr += strlen(params[i]);
                if (i < type->data.function.parameter_count - 1) {
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
            
        case TYPE_NAMED:
            result = strdup(type->data.named.name);
            break;
            
        default:
            result = strdup("unknown");
            break;
    }
    
    return result;
}

// Type inference
Type* type_infer(ASTNode* node) {
    if (node == NULL) return NULL;

    switch (node->type) {
        case NODE_LITERAL:
            switch (node->as.literal.type) {
                case LITERAL_INTEGER:
                    return type_new(TYPE_INTEGER);
                case LITERAL_FLOAT:
                    return type_new(TYPE_FLOAT);
                case LITERAL_STRING:
                    return type_new(TYPE_STRING);
                case LITERAL_BOOLEAN:
                    return type_new(TYPE_BOOLEAN);
                case LITERAL_NULL:
                    return type_new(TYPE_VOID);
            }
            break;

        case NODE_BINARY: {
            Type* left_type = type_infer(node->as.binary.left);
            Type* right_type = type_infer(node->as.binary.right);
            
            if (left_type == NULL || right_type == NULL) {
                type_free(left_type);
                type_free(right_type);
                return NULL;
            }

            // Check operator type
            switch (node->as.binary.operator) {
                case TOKEN_PLUS:
                case TOKEN_MINUS:
                case TOKEN_STAR:
                case TOKEN_SLASH:
                    if (left_type->kind == TYPE_INTEGER && right_type->kind == TYPE_INTEGER) {
                        type_free(right_type);
                        return left_type;
                    }
                    if ((left_type->kind == TYPE_FLOAT || left_type->kind == TYPE_INTEGER) &&
                        (right_type->kind == TYPE_FLOAT || right_type->kind == TYPE_INTEGER)) {
                        type_free(left_type);
                        return right_type;
                    }
                    break;

                case TOKEN_EQ:
                case TOKEN_NEQ:
                case TOKEN_LT:
                case TOKEN_GT:
                case TOKEN_LE:
                case TOKEN_GE:
                    if (type_is_compatible_with(left_type, right_type)) {
                        type_free(left_type);
                        type_free(right_type);
                        return type_new(TYPE_BOOLEAN);
                    }
                    break;
            }

            type_free(left_type);
            type_free(right_type);
            return NULL;
        }

        case NODE_UNARY: {
            Type* operand_type = type_infer(node->as.unary.operand);
            if (operand_type == NULL) return NULL;

            switch (node->as.unary.operator) {
                case TOKEN_MINUS:
                    if (operand_type->kind == TYPE_INTEGER || operand_type->kind == TYPE_FLOAT) {
                        return operand_type;
                    }
                    break;
                case TOKEN_BANG:
                    if (operand_type->kind == TYPE_BOOLEAN) {
                        type_free(operand_type);
                        return type_new(TYPE_BOOLEAN);
                    }
                    break;
            }

            type_free(operand_type);
            return NULL;
        }

        // TODO: Implement type inference for other node types

        default:
            return NULL;
    }

    return NULL;
}

// Type checking
SlangError type_check(ASTNode* node) {
    if (node == NULL) return SLANG_SUCCESS;

    Type* inferred_type = type_infer(node);
    if (inferred_type == NULL) {
        return SLANG_ERROR_TYPE;
    }

    type_free(inferred_type);
    return SLANG_SUCCESS;
} 