#ifndef SLANG_PARSER_H
#define SLANG_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "vector.h"
#include "error.h"
#include "type_system.h"
#include <stdbool.h>

// Parameter structure
typedef struct {
    char* name;
    Type* type_annotation;
} Parameter;

// Parser structure
typedef struct {
    Lexer* lexer;
    Token* current;
    Token* previous;
    Vector* errors;
} Parser;

// Function declarations
Parser* parser_create(Lexer* lexer);
void parser_destroy(Parser* parser);
SlangError parser_parse(Parser* parser, ASTNode** ast);
bool parser_had_error(Parser* parser);
void parser_synchronize(Parser* parser);

// AST manipulation functions
void ast_add_function(AST* ast, Function* function);
void ast_add_type_definition(AST* ast, TypeDefinition* type_def);

// Additional function declarations
SlangError* parser_parse_function(Parser* parser, Function* function);
SlangError* parser_parse_type_definition(Parser* parser, TypeDefinition* type_def);
SlangError* parser_parse_integer(Parser* parser, int* value);
SlangError* parser_parse_block(Parser* parser, ASTNode** block);
SlangError* parser_parse_statement(Parser* parser, ASTNode** statement);
SlangError* parser_parse_expression(Parser* parser, ASTNode** expression);

#endif // SLANG_PARSER_H 