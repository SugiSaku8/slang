#ifndef SLANG_LEXER_H
#define SLANG_LEXER_H

#include <stdbool.h>
#include <stdint.h>

// Token types
typedef enum {
    // Identifiers
    TOKEN_IDENTIFIER,

    // Literals
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_CHAR_LITERAL,

    // Keywords
    TOKEN_FUNCTION,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_RETURN,
    TOKEN_MATCH,
    TOKEN_TYPE,
    TOKEN_PRIORITY,
    TOKEN_MOST_HIGH,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,

    // Operators
    TOKEN_ASSIGN,
    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN,
    TOKEN_LESS_THAN_EQUALS,
    TOKEN_GREATER_THAN_EQUALS,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    // Delimiters
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_ARROW,
    TOKEN_FAT_ARROW,
    TOKEN_UNDERSCORE,

    // Special tokens
    TOKEN_VAR_TYPE_PRIORITY,
    TOKEN_FUNCTION_TYPE_PRIORITY,
    TOKEN_MACRO_TYPE,
    TOKEN_EOF
} TokenType;

// Token value union
typedef union {
    int64_t integer;
    double floating;
    char* string;
    char character;
} TokenValue;

// Token structure
typedef struct {
    TokenType type;
    TokenValue value;
    size_t start;
    size_t end;
} Token;

// Lexer structure
typedef struct {
    const char* source;
    size_t position;
    size_t length;
    Token current_token;
} Lexer;

// Lexer functions
Lexer* lexer_new(const char* source);
void lexer_free(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
bool lexer_is_at_end(Lexer* lexer);

#endif // SLANG_LEXER_H 