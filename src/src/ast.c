#include "ast.h"
#include <stdlib.h>
#include <string.h>

// Vector implementation
struct Vector {
    void* data;
    size_t size;
    size_t capacity;
    size_t element_size;
};

Vector* vector_new(size_t element_size) {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    if (vector == NULL) {
        return NULL;
    }
    
    vector->element_size = element_size;
    vector->size = 0;
    vector->capacity = 16;
    vector->data = malloc(vector->capacity * element_size);
    
    if (vector->data == NULL) {
        free(vector);
        return NULL;
    }
    
    return vector;
}

void vector_free(Vector* vector) {
    if (vector != NULL) {
        free(vector->data);
        free(vector);
    }
}

void vector_push(Vector* vector, const void* element) {
    if (vector->size >= vector->capacity) {
        vector->capacity *= 2;
        void* new_data = realloc(vector->data, vector->capacity * vector->element_size);
        if (new_data == NULL) {
            // Handle error
            return;
        }
        vector->data = new_data;
    }
    
    memcpy((char*)vector->data + vector->size * vector->element_size,
           element,
           vector->element_size);
    vector->size++;
}

// AST implementation
AST* ast_new(void) {
    AST* ast = (AST*)malloc(sizeof(AST));
    if (ast == NULL) {
        return NULL;
    }
    
    ast->functions = vector_new(sizeof(Function));
    ast->type_definitions = vector_new(sizeof(TypeDefinition));
    
    if (ast->functions == NULL || ast->type_definitions == NULL) {
        if (ast->functions != NULL) vector_free(ast->functions);
        if (ast->type_definitions != NULL) vector_free(ast->type_definitions);
        free(ast);
        return NULL;
    }
    
    return ast;
}

void ast_free(AST* ast) {
    if (ast != NULL) {
        // Free functions
        for (size_t i = 0; i < ast->functions->size; i++) {
            Function* func = (Function*)((char*)ast->functions->data + i * sizeof(Function));
            free(func->name);
            vector_free(func->parameters);
            // Free function body
            for (size_t j = 0; j < func->body.statements->size; j++) {
                Statement* stmt = (Statement*)((char*)func->body.statements->data + j * sizeof(Statement));
                // Free statement contents
                switch (stmt->kind) {
                    case STATEMENT_LET:
                        free(stmt->let_statement.name);
                        if (stmt->let_statement.type_annotation != NULL) {
                            free(stmt->let_statement.type_annotation);
                        }
                        free(stmt->let_statement.value);
                        break;
                    case STATEMENT_RETURN:
                        if (stmt->return_statement.value != NULL) {
                            free(stmt->return_statement.value);
                        }
                        break;
                    case STATEMENT_IF:
                        free(stmt->if_statement.condition);
                        // Free then block
                        vector_free(stmt->if_statement.then_block.statements);
                        // Free else block if exists
                        if (stmt->if_statement.else_block != NULL) {
                            vector_free(stmt->if_statement.else_block->statements);
                            free(stmt->if_statement.else_block);
                        }
                        break;
                    case STATEMENT_WHILE:
                        free(stmt->while_statement.condition);
                        vector_free(stmt->while_statement.body.statements);
                        break;
                    case STATEMENT_FOR:
                        free(stmt->for_statement.variable);
                        free(stmt->for_statement.iterator);
                        vector_free(stmt->for_statement.body.statements);
                        break;
                    case STATEMENT_MATCH:
                        free(stmt->match_statement.expression);
                        // Free match arms
                        for (size_t k = 0; k < stmt->match_statement.arms->size; k++) {
                            MatchArm* arm = (MatchArm*)((char*)stmt->match_statement.arms->data + k * sizeof(MatchArm));
                            // Free pattern
                            switch (arm->pattern.kind) {
                                case PATTERN_IDENTIFIER:
                                    free(arm->pattern.identifier);
                                    break;
                                case PATTERN_TUPLE:
                                    vector_free(arm->pattern.tuple_patterns);
                                    break;
                                case PATTERN_STRUCT:
                                    free(arm->pattern.struct_pattern.name);
                                    vector_free(arm->pattern.struct_pattern.fields);
                                    break;
                                default:
                                    break;
                            }
                            vector_free(arm->body.statements);
                        }
                        vector_free(stmt->match_statement.arms);
                        break;
                    case STATEMENT_EXPRESSION:
                        // Free expression
                        switch (stmt->expression_statement.kind) {
                            case EXPRESSION_IDENTIFIER:
                                free(stmt->expression_statement.identifier);
                                break;
                            case EXPRESSION_BINARY_OP:
                                free(stmt->expression_statement.binary_op->left);
                                free(stmt->expression_statement.binary_op->right);
                                free(stmt->expression_statement.binary_op);
                                break;
                            case EXPRESSION_UNARY_OP:
                                free(stmt->expression_statement.unary_op->right);
                                free(stmt->expression_statement.unary_op);
                                break;
                            case EXPRESSION_CALL:
                                free(stmt->expression_statement.call->function);
                                vector_free(stmt->expression_statement.call->arguments);
                                free(stmt->expression_statement.call);
                                break;
                            case EXPRESSION_ASSIGNMENT:
                                free(stmt->expression_statement.assignment->target);
                                free(stmt->expression_statement.assignment->value);
                                free(stmt->expression_statement.assignment);
                                break;
                            default:
                                break;
                        }
                        break;
                }
            }
            vector_free(func->body.statements);
        }
        vector_free(ast->functions);
        
        // Free type definitions
        for (size_t i = 0; i < ast->type_definitions->size; i++) {
            TypeDefinition* type_def = (TypeDefinition*)((char*)ast->type_definitions->data + i * sizeof(TypeDefinition));
            free(type_def->name);
            vector_free(type_def->fields);
        }
        vector_free(ast->type_definitions);
        
        free(ast);
    }
}

void ast_add_function(AST* ast, Function* function) {
    vector_push(ast->functions, function);
}

void ast_add_type_definition(AST* ast, TypeDefinition* type_def) {
    vector_push(ast->type_definitions, type_def);
} 