#include "../include/ast.h"
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
    if (!vector) return NULL;
    
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
    vector->element_size = element_size;
    return vector;
}

void vector_free(Vector* vector) {
    if (!vector) return;
    free(vector->data);
    free(vector);
}

void* vector_push(Vector* vector, const void* element) {
    if (!vector || !element) return NULL;
    
    if (vector->size >= vector->capacity) {
        size_t new_capacity = vector->capacity == 0 ? 1 : vector->capacity * 2;
        void* new_data = realloc(vector->data, new_capacity * vector->element_size);
        if (!new_data) return NULL;
        
        vector->data = new_data;
        vector->capacity = new_capacity;
    }
    
    void* dest = (char*)vector->data + vector->size * vector->element_size;
    memcpy(dest, element, vector->element_size);
    vector->size++;
    return dest;
}

// AST implementation
AST* ast_new(void) {
    AST* ast = (AST*)malloc(sizeof(AST));
    if (!ast) return NULL;
    
    ast->functions = vector_new(sizeof(Function));
    ast->type_definitions = vector_new(sizeof(TypeDefinition));
    
    if (!ast->functions || !ast->type_definitions) {
        if (ast->functions) vector_free(ast->functions);
        if (ast->type_definitions) vector_free(ast->type_definitions);
        free(ast);
        return NULL;
    }
    
    return ast;
}

void ast_free(AST* ast) {
    if (!ast) return;
    
    // Free functions
    for (size_t i = 0; i < ast->functions->size; i++) {
        Function* func = (Function*)((char*)ast->functions->data + i * sizeof(Function));
        
        // Free function name
        free(func->name);
        
        // Free parameters
        for (size_t j = 0; j < func->parameters->size; j++) {
            Parameter* param = (Parameter*)((char*)func->parameters->data + j * sizeof(Parameter));
            free(param->name);
        }
        vector_free(func->parameters);
        
        // Free block statements
        for (size_t j = 0; j < func->body.statements->size; j++) {
            Statement* stmt = (Statement*)((char*)func->body.statements->data + j * sizeof(Statement));
            statement_free(stmt);
        }
        vector_free(func->body.statements);
    }
    vector_free(ast->functions);
    
    // Free type definitions
    for (size_t i = 0; i < ast->type_definitions->size; i++) {
        TypeDefinition* type_def = (TypeDefinition*)((char*)ast->type_definitions->data + i * sizeof(TypeDefinition));
        
        // Free type name
        free(type_def->name);
        
        // Free fields
        for (size_t j = 0; j < type_def->fields->size; j++) {
            Field* field = (Field*)((char*)type_def->fields->data + j * sizeof(Field));
            free(field->name);
        }
        vector_free(type_def->fields);
    }
    vector_free(ast->type_definitions);
    
    free(ast);
}

void ast_add_function(AST* ast, Function* function) {
    if (!ast || !function) return;
    vector_push(ast->functions, function);
}

void ast_add_type_definition(AST* ast, TypeDefinition* type_def) {
    if (!ast || !type_def) return;
    vector_push(ast->type_definitions, type_def);
}

// Statement implementation
void statement_free(Statement* stmt) {
    if (!stmt) return;
    
    switch (stmt->kind) {
        case STATEMENT_LET:
            free(stmt->let_statement.name);
            if (stmt->let_statement.type_annotation) {
                type_free(stmt->let_statement.type_annotation);
            }
            expression_free(stmt->let_statement.value);
            break;
            
        case STATEMENT_RETURN:
            expression_free(stmt->return_statement.value);
            break;
            
        case STATEMENT_IF:
            expression_free(stmt->if_statement.condition);
            // Free then block
            for (size_t i = 0; i < stmt->if_statement.then_block.statements->size; i++) {
                Statement* then_stmt = (Statement*)((char*)stmt->if_statement.then_block.statements->data + i * sizeof(Statement));
                statement_free(then_stmt);
            }
            vector_free(stmt->if_statement.then_block.statements);
            
            // Free else block if exists
            if (stmt->if_statement.else_block) {
                for (size_t i = 0; i < stmt->if_statement.else_block->statements->size; i++) {
                    Statement* else_stmt = (Statement*)((char*)stmt->if_statement.else_block->statements->data + i * sizeof(Statement));
                    statement_free(else_stmt);
                }
                vector_free(stmt->if_statement.else_block->statements);
                free(stmt->if_statement.else_block);
            }
            break;
            
        case STATEMENT_WHILE:
            expression_free(stmt->while_statement.condition);
            // Free body
            for (size_t i = 0; i < stmt->while_statement.body.statements->size; i++) {
                Statement* body_stmt = (Statement*)((char*)stmt->while_statement.body.statements->data + i * sizeof(Statement));
                statement_free(body_stmt);
            }
            vector_free(stmt->while_statement.body.statements);
            break;
            
        case STATEMENT_FOR:
            free(stmt->for_statement.variable);
            expression_free(stmt->for_statement.iterator);
            // Free body
            for (size_t i = 0; i < stmt->for_statement.body.statements->size; i++) {
                Statement* body_stmt = (Statement*)((char*)stmt->for_statement.body.statements->data + i * sizeof(Statement));
                statement_free(body_stmt);
            }
            vector_free(stmt->for_statement.body.statements);
            break;
            
        case STATEMENT_MATCH:
            expression_free(stmt->match_statement.expression);
            // Free arms
            for (size_t i = 0; i < stmt->match_statement.arms->size; i++) {
                MatchArm* arm = (MatchArm*)((char*)stmt->match_statement.arms->data + i * sizeof(MatchArm));
                pattern_free(&arm->pattern);
                // Free body
                for (size_t j = 0; j < arm->body.statements->size; j++) {
                    Statement* body_stmt = (Statement*)((char*)arm->body.statements->data + j * sizeof(Statement));
                    statement_free(body_stmt);
                }
                vector_free(arm->body.statements);
            }
            vector_free(stmt->match_statement.arms);
            break;
            
        case STATEMENT_EXPRESSION:
            expression_free(&stmt->expression_statement);
            break;
    }
}

// Pattern implementation
void pattern_free(Pattern* pattern) {
    if (!pattern) return;
    
    switch (pattern->kind) {
        case PATTERN_IDENTIFIER:
            free(pattern->identifier);
            break;
            
        case PATTERN_LITERAL:
            literal_free(&pattern->literal);
            break;
            
        case PATTERN_TUPLE:
            for (size_t i = 0; i < pattern->tuple_patterns->size; i++) {
                Pattern* tuple_pattern = (Pattern*)((char*)pattern->tuple_patterns->data + i * sizeof(Pattern));
                pattern_free(tuple_pattern);
            }
            vector_free(pattern->tuple_patterns);
            break;
            
        case PATTERN_STRUCT:
            free(pattern->struct_pattern.name);
            for (size_t i = 0; i < pattern->struct_pattern.fields->size; i++) {
                FieldPattern* field = (FieldPattern*)((char*)pattern->struct_pattern.fields->data + i * sizeof(FieldPattern));
                free(field->name);
                pattern_free(field->pattern);
            }
            vector_free(pattern->struct_pattern.fields);
            break;
            
        default:
            break;
    }
}

// Expression implementation
void expression_free(Expression* expr) {
    if (!expr) return;
    
    switch (expr->kind) {
        case EXPRESSION_LITERAL:
            literal_free(&expr->literal);
            break;
            
        case EXPRESSION_IDENTIFIER:
            free(expr->identifier);
            break;
            
        case EXPRESSION_BINARY_OP:
            expression_free(expr->binary_op->left);
            expression_free(expr->binary_op->right);
            free(expr->binary_op);
            break;
            
        case EXPRESSION_UNARY_OP:
            expression_free(expr->unary_op->right);
            free(expr->unary_op);
            break;
            
        case EXPRESSION_CALL:
            free(expr->call->function);
            for (size_t i = 0; i < expr->call->arguments->size; i++) {
                Expression* arg = (Expression*)((char*)expr->call->arguments->data + i * sizeof(Expression));
                expression_free(arg);
            }
            vector_free(expr->call->arguments);
            free(expr->call);
            break;
            
        case EXPRESSION_ASSIGNMENT:
            free(expr->assignment->target);
            expression_free(expr->assignment->value);
            free(expr->assignment);
            break;
    }
}

// Literal implementation
void literal_free(Literal* literal) {
    if (!literal) return;
    
    switch (literal->kind) {
        case LITERAL_STRING:
            free(literal->string);
            break;
            
        default:
            break;
    }
} 