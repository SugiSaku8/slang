#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "ast.h"
#include "type_system.h"

typedef struct {
    struct {
        char** keys;
        Type** values;
        size_t count;
        size_t capacity;
    } variables;
    struct {
        char** keys;
        Type** values;
        size_t count;
        size_t capacity;
    } functions;
} TypeChecker;

// Function declarations
TypeChecker* create_type_checker(void);
void free_type_checker(TypeChecker* type_checker);
Type* check_expression(TypeChecker* type_checker, ASTNode* node);
void check_statement(TypeChecker* type_checker, ASTNode* node);
void declare_variable_type(TypeChecker* type_checker, const char* name, Type* type);
Type* get_variable_type(TypeChecker* type_checker, const char* name);
void declare_function_type(TypeChecker* type_checker, const char* name, Type* type);
Type* get_function_type(TypeChecker* type_checker, const char* name);

#endif // TYPE_CHECKER_H 