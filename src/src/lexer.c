#include "../include/lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

// Helper functions
static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

static bool is_identifier_start(char c) {
    return isalpha(c) || c == '_';
}

static bool is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

static bool is_digit(char c) {
    return isdigit(c);
}

static bool is_hex_digit(char c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool is_at_end(Lexer* lexer) {
    return lexer->current >= strlen(lexer->source);
}

static char advance(Lexer* lexer) {
    return lexer->source[lexer->current++];
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;
    lexer->current++;
    return true;
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current + 1];
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) advance(lexer);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->source + lexer->start;
    token.length = lexer->current - lexer->start;
    token.line = lexer->line;
    return token;
}

static Token error_token(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = strlen(message);
    token.line = lexer->line;
    return token;
}

static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string.");
    }

    // The closing quote
    advance(lexer);

    // Trim the surrounding quotes
    size_t length = lexer->current - lexer->start - 2;
    char* str = malloc(length + 1);
    strncpy(str, lexer->source + lexer->start + 1, length);
    str[length] = '\0';

    Token token = make_token(lexer, TOKEN_STRING);
    token.value.string = str;
    return token;
}

static Token number(Lexer* lexer) {
    while (isdigit(peek(lexer))) advance(lexer);

    // Look for a fractional part
    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        // Consume the "."
        advance(lexer);

        while (isdigit(peek(lexer))) advance(lexer);
    }

    Token token = make_token(lexer, TOKEN_NUMBER);
    char* num_str = malloc(lexer->current - lexer->start + 1);
    strncpy(num_str, lexer->source + lexer->start, lexer->current - lexer->start);
    num_str[lexer->current - lexer->start] = '\0';
    token.value.number = atof(num_str);
    free(num_str);
    return token;
}

static TokenType check_keyword(Lexer* lexer, int start, int length, const char* rest, TokenType type) {
    if (lexer->current - lexer->start == start + length &&
        memcmp(lexer->source + lexer->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Lexer* lexer) {
    switch (lexer->source[lexer->start]) {
        case 'a': return check_keyword(lexer, 1, 2, "nd", TOKEN_AND);
        case 'c': return check_keyword(lexer, 1, 4, "lass", TOKEN_CLASS);
        case 'e': return check_keyword(lexer, 1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'a': return check_keyword(lexer, 2, 3, "lse", TOKEN_FALSE);
                    case 'o': return check_keyword(lexer, 2, 1, "r", TOKEN_FOR);
                    case 'n': return check_keyword(lexer, 2, 0, "", TOKEN_FUNCTION);
                }
            }
            break;
        case 'i':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'f': return check_keyword(lexer, 2, 0, "", TOKEN_IF);
                    case 'n': return check_keyword(lexer, 2, 0, "", TOKEN_IN);
                }
            }
            break;
        case 'm': return check_keyword(lexer, 1, 4, "atch", TOKEN_MATCH);
        case 'n': return check_keyword(lexer, 1, 3, "ull", TOKEN_NULL);
        case 'o': return check_keyword(lexer, 1, 1, "r", TOKEN_OR);
        case 'p': return check_keyword(lexer, 1, 4, "rint", TOKEN_PRINT);
        case 'r': return check_keyword(lexer, 1, 5, "eturn", TOKEN_RETURN);
        case 's': return check_keyword(lexer, 1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'h': return check_keyword(lexer, 2, 2, "is", TOKEN_THIS);
                    case 'r': return check_keyword(lexer, 2, 2, "ue", TOKEN_TRUE);
                    case 'y': return check_keyword(lexer, 2, 2, "pe", TOKEN_TYPE);
                }
            }
            break;
        case 'v': return check_keyword(lexer, 1, 2, "ar", TOKEN_VAR);
        case 'w': return check_keyword(lexer, 1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer* lexer) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') advance(lexer);
    Token token = make_token(lexer, identifier_type(lexer));
    return token;
}

Token scan_token(Lexer* lexer) {
    skip_whitespace(lexer);
    lexer->start = lexer->current;
    Token token = {0};

    if (is_at_end(lexer)) {
        token.type = TOKEN_EOF;
        return token;
    }

    char c = advance(lexer);
    if (isalpha(c) || c == '_') {
        return identifier(lexer);
    }
    if (isdigit(c)) {
        return number(lexer);
    }

    switch (c) {
        case '(': token.type = TOKEN_LEFT_PAREN; break;
        case ')': token.type = TOKEN_RIGHT_PAREN; break;
        case '{': token.type = TOKEN_LEFT_BRACE; break;
        case '}': token.type = TOKEN_RIGHT_BRACE; break;
        case '[': token.type = TOKEN_LEFT_BRACKET; break;
        case ']': token.type = TOKEN_RIGHT_BRACKET; break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '.': token.type = TOKEN_DOT; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '+': token.type = TOKEN_PLUS; break;
        case '/': token.type = TOKEN_SLASH; break;
        case '*': token.type = TOKEN_STAR; break;
        case '!': token.type = match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG; break;
        case '=': token.type = match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL; break;
        case '<': token.type = match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS; break;
        case '>': token.type = match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER; break;
        case '"': return string(lexer);
        default: token.type = TOKEN_ERROR; break;
    }

    return token;
}

Lexer* lexer_init(const char* source) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    return lexer;
}

void lexer_free(Lexer* lexer) {
    free(lexer);
}

Token lexer_next_token(Lexer* lexer) {
    return scan_token(lexer);
}

Token lexer_peek_token(Lexer* lexer) {
    size_t current_position = lexer->current;
    Token token = lexer_next_token(lexer);
    lexer->current = current_position;
    return token;
}

bool lexer_is_at_end(Lexer* lexer) {
    return lexer->source[lexer->current] == '\0';
} 