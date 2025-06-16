#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

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

#endif // PARSER_H 