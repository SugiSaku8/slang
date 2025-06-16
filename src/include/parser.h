#ifndef SLANG_PARSER_H
#define SLANG_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "error.h"
#include <stdbool.h>

typedef struct {
    Lexer* lexer;
} Parser;

// Parser functions
Parser* parser_new(Lexer* lexer);
void parser_free(Parser* parser);
SlangError* parser_parse(Parser* parser, AST* ast);

// Helper functions
SlangError* parser_parse_function(Parser* parser, Function* function);
SlangError* parser_parse_type(Parser* parser, Type* type);
SlangError* parser_parse_type_definition(Parser* parser, TypeDefinition* type_def);
SlangError* parser_parse_pattern(Parser* parser, Pattern* pattern);
SlangError* parser_parse_identifier(Parser* parser, char** identifier);
SlangError* parser_parse_string(Parser* parser, char** string);
SlangError* parser_parse_integer(Parser* parser, int32_t* integer);
SlangError* parser_parse_float(Parser* parser, double* floating);
SlangError* parser_parse_block(Parser* parser, Block* block);
SlangError* parser_parse_statement(Parser* parser, Statement* statement);
SlangError* parser_parse_expression(Parser* parser, Expression* expression);
SlangError* parser_expect(Parser* parser, TokenType expected);

#endif // SLANG_PARSER_H 