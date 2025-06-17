#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "vector.h"
#include "error.h"
#include "type_system.h"
#include <stdbool.h>

typedef struct {
    Lexer* lexer;
    Token current_token;
    Token previous_token;
    bool had_error;
} Parser;

// Function declarations
Parser* create_parser(Lexer* lexer);
void free_parser(Parser* parser);
ASTNode* parse_program(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_primary(Parser* parser);
ASTNode* parse_function_declaration(Parser* parser);
ASTNode* parse_let_statement(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_while_statement(Parser* parser);
ASTNode* parse_block_statement(Parser* parser);
ASTNode* parse_expression_statement(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
ASTNode* parse_call_expression(Parser* parser);
ASTNode* parse_binary_expression(Parser* parser, int precedence);
ASTNode* parse_unary_expression(Parser* parser);

// Additional function declarations
SlangError* parser_parse(Parser* parser, AST* ast);
SlangError* parser_parse_function(Parser* parser, Function* function);
SlangError* parser_parse_type(Parser* parser, Type* type);
SlangError* parser_parse_type_definition(Parser* parser, TypeDefinition* type_def);
SlangError* parser_parse_identifier(Parser* parser, char** name);
SlangError* parser_parse_integer(Parser* parser, int* value);
SlangError* parser_parse_block(Parser* parser, ASTNode** block);
SlangError* parser_expect(Parser* parser, TokenType expected);

#endif // PARSER_H 