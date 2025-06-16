#ifndef SLANG_AST_H
#define SLANG_AST_H

#include "type_system.h"
#include <stdbool.h>

// Forward declarations
typedef struct Vector Vector;
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

// AST structure
struct AST {
    Vector* functions;
    Vector* type_definitions;
};

// Function structure
struct Function {
    char* name;
    Vector* parameters;
    Type return_type;
    int32_t priority;
    Block body;
};

// Parameter structure
struct Parameter {
    char* name;
    Type type_annotation;
};

// TypeDefinition structure
struct TypeDefinition {
    char* name;
    Vector* fields;
};

// Field structure
struct Field {
    char* name;
    Type type_annotation;
};

// Block structure
struct Block {
    Vector* statements;
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
        LetStatement let_statement;
        ReturnStatement return_statement;
        IfStatement if_statement;
        WhileStatement while_statement;
        ForStatement for_statement;
        MatchStatement match_statement;
        Expression expression_statement;
    };
};

// LetStatement structure
struct LetStatement {
    char* name;
    Type* type_annotation;
    Expression* value;
};

// ReturnStatement structure
struct ReturnStatement {
    Expression* value;
};

// IfStatement structure
struct IfStatement {
    Expression* condition;
    Block then_block;
    Block* else_block;
};

// WhileStatement structure
struct WhileStatement {
    Expression* condition;
    Block body;
};

// ForStatement structure
struct ForStatement {
    char* variable;
    Expression* iterator;
    Block body;
};

// MatchStatement structure
struct MatchStatement {
    Expression* expression;
    Vector* arms;
};

// MatchArm structure
struct MatchArm {
    Pattern pattern;
    Block body;
};

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
        Literal literal;
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
    Pattern* pattern;
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

// Expression structure
struct Expression {
    ExpressionType kind;
    union {
        Literal literal;
        char* identifier;
        BinaryOpExpression* binary_op;
        UnaryOpExpression* unary_op;
        CallExpression* call;
        AssignmentExpression* assignment;
    };
};

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

// BinaryOpExpression structure
struct BinaryOpExpression {
    Expression* left;
    BinaryOperator op;
    Expression* right;
};

// UnaryOperator types
typedef enum {
    UNARY_OP_NEG,
    UNARY_OP_NOT
} UnaryOperator;

// UnaryOpExpression structure
struct UnaryOpExpression {
    UnaryOperator op;
    Expression* right;
};

// CallExpression structure
struct CallExpression {
    char* function;
    Vector* arguments;
};

// AssignmentExpression structure
struct AssignmentExpression {
    char* target;
    Expression* value;
};

// AST functions
AST* ast_new(void);
void ast_free(AST* ast);
void ast_add_function(AST* ast, Function* function);
void ast_add_type_definition(AST* ast, TypeDefinition* type_def);

#endif // SLANG_AST_H 