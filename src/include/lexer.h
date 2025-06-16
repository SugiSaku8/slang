#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_LET,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_LESS,
    TOKEN_LESS_EQUALS,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUALS,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_DOT,
    TOKEN_ARROW,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char* lexeme;
    int64_t line;
    int64_t column;
    union {
        int64_t integer_value;
        double float_value;
        char* string_value;
        char* error_message;
    } literal;
} Token;

typedef struct {
    const char* source;
    size_t start;
    size_t current;
    int64_t line;
    int64_t column;
} Lexer;

// Function declarations
Lexer* create_lexer(const char* source);
void free_lexer(Lexer* lexer);
Token next_token(Lexer* lexer);
Token peek_token(Lexer* lexer);
bool is_at_end(Lexer* lexer);

#endif // LEXER_H 