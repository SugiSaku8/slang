#ifndef AST_H
#define AST_H

#include "type_system.h"
#include <stdbool.h>
#include <stdint.h>

// Type definitions
typedef struct {
    char* name;
    Type* type;
} TypeDefinition;

typedef struct {
    char* name;
    Type* type;
} Variable;

typedef struct {
    char* name;
    Type* return_type;
    Variable** parameters;
    size_t parameter_count;
    struct ASTNode* body;
    int priority;
} Function;

typedef struct {
    Function** functions;
    size_t function_count;
    TypeDefinition** type_definitions;
    size_t type_definition_count;
} AST;

typedef struct {
    char* name;
    Type* type;
    struct ASTNode* initializer;
} LetStatement;

typedef struct {
    struct ASTNode* condition;
    struct ASTNode* then_branch;
    struct ASTNode* else_branch;
} IfStatement;

typedef struct {
    struct ASTNode* condition;
    struct ASTNode* body;
} WhileStatement;

typedef struct {
    struct ASTNode* callee;
    struct ASTNode** arguments;
    size_t argument_count;
} CallExpression;

typedef struct {
    char* name;
    struct ASTNode** arguments;
    size_t argument_count;
} FunctionCall;

typedef struct {
    char* name;
    struct ASTNode* value;
} Assignment;

typedef struct {
    char* name;
} VariableReference;

typedef struct {
    int64_t value;
} IntegerLiteral;

typedef struct {
    double value;
} FloatLiteral;

typedef struct {
    char* value;
} StringLiteral;

typedef struct {
    bool value;
} BooleanLiteral;

typedef struct {
    struct ASTNode* left;
    char* operator;
    struct ASTNode* right;
} BinaryExpression;

typedef struct {
    char* operator;
    struct ASTNode* right;
} UnaryExpression;

typedef struct {
    struct ASTNode* expression;
} ExpressionStatement;

typedef struct {
    struct ASTNode** statements;
    size_t statement_count;
} BlockStatement;

typedef struct ASTNode {
    enum {
        NODE_VARIABLE,
        NODE_FUNCTION,
        NODE_LET_STATEMENT,
        NODE_IF_STATEMENT,
        NODE_WHILE_STATEMENT,
        NODE_CALL_EXPRESSION,
        NODE_FUNCTION_CALL,
        NODE_ASSIGNMENT,
        NODE_VARIABLE_REFERENCE,
        NODE_INTEGER_LITERAL,
        NODE_FLOAT_LITERAL,
        NODE_STRING_LITERAL,
        NODE_BOOLEAN_LITERAL,
        NODE_BINARY_EXPRESSION,
        NODE_UNARY_EXPRESSION,
        NODE_EXPRESSION_STATEMENT,
        NODE_BLOCK_STATEMENT
    } type;
    union {
        Variable variable;
        Function function;
        LetStatement let_statement;
        IfStatement if_statement;
        WhileStatement while_statement;
        CallExpression call_expression;
        FunctionCall function_call;
        Assignment assignment;
        VariableReference variable_reference;
        IntegerLiteral integer_literal;
        FloatLiteral float_literal;
        StringLiteral string_literal;
        BooleanLiteral boolean_literal;
        BinaryExpression binary_expression;
        UnaryExpression unary_expression;
        ExpressionStatement expression_statement;
        BlockStatement block_statement;
    } data;
} ASTNode;

// Function declarations
AST* create_ast(void);
void free_ast(AST* ast);
void ast_add_function(AST* ast, Function* function);
void ast_add_type_definition(AST* ast, TypeDefinition* type_def);

ASTNode* create_variable_node(const char* name, Type* type);
ASTNode* create_function_node(const char* name, Type* return_type, Variable** parameters, size_t parameter_count, ASTNode* body);
ASTNode* create_let_statement_node(const char* name, Type* type, ASTNode* initializer);
ASTNode* create_if_statement_node(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* create_while_statement_node(ASTNode* condition, ASTNode* body);
ASTNode* create_call_expression_node(ASTNode* callee, ASTNode** arguments, size_t argument_count);
ASTNode* create_function_call_node(const char* name, ASTNode** arguments, size_t argument_count);
ASTNode* create_assignment_node(const char* name, ASTNode* value);
ASTNode* create_variable_reference_node(const char* name);
ASTNode* create_integer_literal_node(int64_t value);
ASTNode* create_float_literal_node(double value);
ASTNode* create_string_literal_node(const char* value);
ASTNode* create_boolean_literal_node(bool value);
ASTNode* create_binary_expression_node(ASTNode* left, const char* operator, ASTNode* right);
ASTNode* create_unary_expression_node(const char* operator, ASTNode* right);
ASTNode* create_expression_statement_node(ASTNode* expression);
ASTNode* create_block_statement_node(ASTNode** statements, size_t statement_count);

void free_ast_node(ASTNode* node);

#endif // AST_H 