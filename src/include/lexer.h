#ifndef SLANG_LEXER_H
#define SLANG_LEXER_H

#include "common.h"

// トークンの種類
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
    TOKEN_SLASH,     // /
    TOKEN_PERCENT,   // %
    TOKEN_EQUAL,     // =
    TOKEN_EQ,        // ==
    TOKEN_NEQ,       // !=
    TOKEN_LT,        // <
    TOKEN_GT,        // >
    TOKEN_LE,        // <=
    TOKEN_GE,        // >=
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA,     // ,
    TOKEN_DOT,       // .
    TOKEN_ARROW,     // ->
    TOKEN_FN,        // fn
    TOKEN_LET,       // let
    TOKEN_IF,        // if
    TOKEN_ELSE,      // else
    TOKEN_WHILE,     // while
    TOKEN_FOR,       // for
    TOKEN_RETURN,    // return
    TOKEN_BREAK,     // break
    TOKEN_CONTINUE,  // continue
    TOKEN_STRUCT,    // struct
    TOKEN_IMPL,      // impl
    TOKEN_TRAIT,     // trait
    TOKEN_USE,       // use
    TOKEN_PUB,       // pub
    TOKEN_PRIV,      // priv
    TOKEN_ERROR
} TokenType;

// トークンの構造体
typedef struct {
    TokenType type;
    char* lexeme;
    size_t line;
    size_t column;
} Token;

// 字句解析器の構造体
typedef struct {
    const char* source;
    size_t start;
    size_t current;
    size_t line;
    size_t column;
    Vector* tokens;
} Lexer;

// 字句解析器の関数
Lexer* lexer_create(const char* source);
void lexer_destroy(Lexer* lexer);
SlangError lexer_scan(Lexer* lexer);
Token* lexer_next_token(Lexer* lexer);
Token* lexer_peek_token(Lexer* lexer);

#endif // SLANG_LEXER_H 