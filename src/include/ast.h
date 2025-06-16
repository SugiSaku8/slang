#ifndef SLANG_AST_H
#define SLANG_AST_H

#include "type_system.h"
#include <stdbool.h>
#include <stddef.h>

// Vector definition
typedef struct Vector {
    void* data;
    size_t size;
    size_t capacity;
    size_t element_size;
} Vector;

// Forward declarations
typedef struct AST AST;
typedef struct Function Function;
typedef struct Parameter Parameter;
typedef struct TypeDefinition TypeDefinition;
typedef struct Field Field;
typedef struct Block Block;
typedef struct Statement Statement;
typedef struct LetStatement LetStatement;
typedef struct ReturnStatement ReturnStatement;
typedef struct IfStatement IfStatement;
typedef struct WhileStatement WhileStatement;
typedef struct ForStatement ForStatement;
typedef struct MatchStatement MatchStatement;
typedef struct MatchArm MatchArm;
typedef struct Pattern Pattern;
typedef struct FieldPattern FieldPattern;
typedef struct Expression Expression;
typedef struct Literal Literal;
typedef struct BinaryOpExpression BinaryOpExpression;
typedef struct UnaryOpExpression UnaryOpExpression;
typedef struct CallExpression CallExpression;
typedef struct AssignmentExpression AssignmentExpression;

// Function declarations
void statement_free(Statement* stmt);
void pattern_free(Pattern* pattern);
void expression_free(Expression* expr);
void literal_free(Literal* literal);

// Vector API
Vector* vector_new(size_t element_size);
void vector_free(Vector* vector);
void* vector_push(Vector* vector, const void* element);

// Literal types
typedef enum {
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_BOOL,
    LITERAL_STRING,
    LITERAL_NULL
} LiteralType;

// Literal structure
struct Literal {
    LiteralType kind;
    union {
        int64_t integer;
        double floating;
        bool boolean;
        char* string;
    };
};

// BinaryOperator types
typedef enum {
    BINARY_OP_ADD,
    BINARY_OP_SUB,
    BINARY_OP_MUL,
    BINARY_OP_DIV,
    BINARY_OP_MOD,
    BINARY_OP_EQ,
    BINARY_OP_NEQ,
    BINARY_OP_LT,
    BINARY_OP_LTE,
    BINARY_OP_GT,
    BINARY_OP_GTE,
    BINARY_OP_AND,
    BINARY_OP_OR
} BinaryOperator;

// UnaryOperator types
typedef enum {
    UNARY_OP_NEG,
    UNARY_OP_NOT
} UnaryOperator;

// Pattern types
typedef enum {
    PATTERN_IDENTIFIER,
    PATTERN_LITERAL,
    PATTERN_WILDCARD,
    PATTERN_TUPLE,
    PATTERN_STRUCT
} PatternType;

// Pattern structure
struct Pattern {
    PatternType kind;
    union {
        char* identifier;
        struct Literal literal;
        Vector* tuple_patterns;
        struct {
            char* name;
            Vector* fields;
        } struct_pattern;
    };
};

// FieldPattern structure
struct FieldPattern {
    char* name;
    struct Pattern* pattern;
};

// Expression types
typedef enum {
    EXPRESSION_LITERAL,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_BINARY_OP,
    EXPRESSION_UNARY_OP,
    EXPRESSION_CALL,
    EXPRESSION_ASSIGNMENT
} ExpressionType;

// BinaryOpExpression structure
struct BinaryOpExpression {
    struct Expression* left;
    BinaryOperator op;
    struct Expression* right;
};

// UnaryOpExpression structure
struct UnaryOpExpression {
    UnaryOperator op;
    struct Expression* right;
};

// CallExpression structure
struct CallExpression {
    char* function;
    Vector* arguments;
};

// AssignmentExpression structure
struct AssignmentExpression {
    char* target;
    struct Expression* value;
};

// Expression structure
struct Expression {
    ExpressionType kind;
    union {
        struct Literal literal;
        char* identifier;
        struct BinaryOpExpression* binary_op;
        struct UnaryOpExpression* unary_op;
        struct CallExpression* call;
        struct AssignmentExpression* assignment;
    };
};

// Block structure
struct Block {
    Vector* statements;
};

// LetStatement structure
struct LetStatement {
    char* name;
    Type* type_annotation;
    struct Expression* value;
};

// ReturnStatement structure
struct ReturnStatement {
    struct Expression* value;
};

// IfStatement structure
struct IfStatement {
    struct Expression* condition;
    struct Block then_block;
    struct Block* else_block;
};

// WhileStatement structure
struct WhileStatement {
    struct Expression* condition;
    struct Block body;
};

// ForStatement structure
struct ForStatement {
    char* variable;
    struct Expression* iterator;
    struct Block body;
};

// MatchStatement structure
struct MatchStatement {
    struct Expression* expression;
    Vector* arms;
};

// MatchArm structure
struct MatchArm {
    struct Pattern pattern;
    struct Block body;
};

// Statement types
typedef enum {
    STATEMENT_LET,
    STATEMENT_RETURN,
    STATEMENT_IF,
    STATEMENT_WHILE,
    STATEMENT_FOR,
    STATEMENT_MATCH,
    STATEMENT_EXPRESSION
} StatementType;

// Statement structure
struct Statement {
    StatementType kind;
    union {
        struct LetStatement let_statement;
        struct ReturnStatement return_statement;
        struct IfStatement if_statement;
        struct WhileStatement while_statement;
        struct ForStatement for_statement;
        struct MatchStatement match_statement;
        struct Expression expression_statement;
    };
};

// Parameter structure
struct Parameter {
    char* name;
    Type type_annotation;
};

// Function structure
struct Function {
    char* name;
    Vector* parameters;
    Type return_type;
    int32_t priority;
    struct Block body;
};

// Field structure
struct Field {
    char* name;
    Type type_annotation;
};

// TypeDefinition structure
struct TypeDefinition {
    char* name;
    Vector* fields;
};

// AST structure
struct AST {
    Vector* functions;
    Vector* type_definitions;
};

// AST functions
AST* ast_new(void);
void ast_free(AST* ast);
void ast_add_function(AST* ast, Function* function);
void ast_add_type_definition(AST* ast, TypeDefinition* type_def);

#endif // SLANG_AST_H 