#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "vector.h"
#include "error.h"
#include "type_system.h"
#include <stdbool.h>

// Token definitions
#define TOKEN_LPAREN 1
#define TOKEN_RPAREN 2

// Parameter structure
typedef struct {
    char* name;
    Type* type_annotation;
} Parameter;

// Parser structure
typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

// Function declarations
Parser* parser_new(Lexer* lexer);
void parser_free(Parser* parser);
AST* parser_parse(Parser* parser);
SlangError* parser_parse_type(Parser* parser, Type* type);
SlangError* parser_parse_identifier(Parser* parser, char** name);
SlangError* parser_expect(Parser* parser, TokenType type);

// AST manipulation functions
void ast_add_function(AST* ast, Function* function);
void ast_add_type_definition(AST* ast, TypeDefinition* type_def);

// Additional function declarations
SlangError* parser_parse_function(Parser* parser, Function* function);
SlangError* parser_parse_type_definition(Parser* parser, TypeDefinition* type_def);
SlangError* parser_parse_integer(Parser* parser, int* value);
SlangError* parser_parse_block(Parser* parser, ASTNode** block);

#endif // PARSER_H 