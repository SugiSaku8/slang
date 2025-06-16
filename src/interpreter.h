#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "type_system.h"

typedef struct {
    struct {
        char** keys;
        void** values;
        size_t count;
        size_t capacity;
    } variables;
    struct {
        char** keys;
        ASTNode** values;
        size_t count;
        size_t capacity;
    } functions;
} Interpreter;

// Function declarations
Interpreter* create_interpreter(void);
void free_interpreter(Interpreter* interpreter);
void interpret(Interpreter* interpreter, ASTNode* node);
void* evaluate_expression(Interpreter* interpreter, ASTNode* node);
void execute_statement(Interpreter* interpreter, ASTNode* node);
void declare_variable(Interpreter* interpreter, const char* name, void* value);
void* get_variable(Interpreter* interpreter, const char* name);
void declare_function(Interpreter* interpreter, const char* name, ASTNode* function_node);
ASTNode* get_function(Interpreter* interpreter, const char* name);

#endif // INTERPRETER_H 