#include "../include/interpreter.h"

Interpreter* create_interpreter(void) { return NULL; }
void free_interpreter(Interpreter* interpreter) {}
void interpret(Interpreter* interpreter, ASTNode* node) {}
void* evaluate_expression(Interpreter* interpreter, ASTNode* node) { return NULL; }
void execute_statement(Interpreter* interpreter, ASTNode* node) {}
void declare_variable(Interpreter* interpreter, const char* name, void* value) {}
void* get_variable(Interpreter* interpreter, const char* name) { return NULL; }
void declare_function(Interpreter* interpreter, const char* name, ASTNode* function_node) {}
ASTNode* get_function(Interpreter* interpreter, const char* name) { return NULL; } 