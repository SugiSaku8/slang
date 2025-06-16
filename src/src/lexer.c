#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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

// Lexer implementation
Lexer* lexer_new(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (lexer == NULL) {
        return NULL;
    }

    lexer->source = source;
    lexer->position = 0;
    lexer->length = strlen(source);
    lexer->current_token.type = TOKEN_EOF;
    lexer->current_token.start = 0;
    lexer->current_token.end = 0;

    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (lexer != NULL) {
        if (lexer->current_token.type == TOKEN_STRING_LITERAL ||
            lexer->current_token.type == TOKEN_IDENTIFIER) {
            free(lexer->current_token.value.string);
        }
        free(lexer);
    }
}

static void skip_whitespace(Lexer* lexer) {
    while (lexer->position < lexer->length && is_whitespace(lexer->source[lexer->position])) {
        lexer->position++;
    }
}

static void skip_comment(Lexer* lexer) {
    if (lexer->position + 1 < lexer->length &&
        lexer->source[lexer->position] == '/' &&
        lexer->source[lexer->position + 1] == '/') {
        lexer->position += 2;
        while (lexer->position < lexer->length && lexer->source[lexer->position] != '\n') {
            lexer->position++;
        }
    }
}

static Token scan_identifier(Lexer* lexer) {
    Token token;
    token.start = lexer->position;
    
    while (lexer->position < lexer->length && is_identifier_char(lexer->source[lexer->position])) {
        lexer->position++;
    }
    
    token.end = lexer->position;
    size_t length = token.end - token.start;
    char* identifier = (char*)malloc(length + 1);
    strncpy(identifier, lexer->source + token.start, length);
    identifier[length] = '\0';
    
    // Check for keywords
    if (strcmp(identifier, "fn") == 0) token.type = TOKEN_FUNCTION;
    else if (strcmp(identifier, "let") == 0) token.type = TOKEN_LET;
    else if (strcmp(identifier, "if") == 0) token.type = TOKEN_IF;
    else if (strcmp(identifier, "else") == 0) token.type = TOKEN_ELSE;
    else if (strcmp(identifier, "while") == 0) token.type = TOKEN_WHILE;
    else if (strcmp(identifier, "for") == 0) token.type = TOKEN_FOR;
    else if (strcmp(identifier, "in") == 0) token.type = TOKEN_IN;
    else if (strcmp(identifier, "return") == 0) token.type = TOKEN_RETURN;
    else if (strcmp(identifier, "match") == 0) token.type = TOKEN_MATCH;
    else if (strcmp(identifier, "type") == 0) token.type = TOKEN_TYPE;
    else if (strcmp(identifier, "priority") == 0) token.type = TOKEN_PRIORITY;
    else if (strcmp(identifier, "most_high") == 0) token.type = TOKEN_MOST_HIGH;
    else if (strcmp(identifier, "true") == 0) token.type = TOKEN_TRUE;
    else if (strcmp(identifier, "false") == 0) token.type = TOKEN_FALSE;
    else if (strcmp(identifier, "null") == 0) token.type = TOKEN_NULL;
    else {
        token.type = TOKEN_IDENTIFIER;
        token.value.string = identifier;
        return token;
    }
    
    free(identifier);
    return token;
}

static Token scan_number(Lexer* lexer) {
    Token token;
    token.start = lexer->position;
    bool is_float = false;
    
    while (lexer->position < lexer->length && is_digit(lexer->source[lexer->position])) {
        lexer->position++;
    }
    
    if (lexer->position < lexer->length && lexer->source[lexer->position] == '.') {
        is_float = true;
        lexer->position++;
        
        while (lexer->position < lexer->length && is_digit(lexer->source[lexer->position])) {
            lexer->position++;
        }
    }
    
    token.end = lexer->position;
    size_t length = token.end - token.start;
    char* number_str = (char*)malloc(length + 1);
    strncpy(number_str, lexer->source + token.start, length);
    number_str[length] = '\0';
    
    if (is_float) {
        token.type = TOKEN_FLOAT_LITERAL;
        token.value.floating = atof(number_str);
    } else {
        token.type = TOKEN_INTEGER_LITERAL;
        token.value.integer = atoll(number_str);
    }
    
    free(number_str);
    return token;
}

static Token scan_string(Lexer* lexer) {
    Token token;
    token.start = lexer->position;
    lexer->position++; // Skip opening quote
    
    while (lexer->position < lexer->length && lexer->source[lexer->position] != '"') {
        if (lexer->source[lexer->position] == '\\') {
            lexer->position++; // Skip escape character
        }
        lexer->position++;
    }
    
    if (lexer->position >= lexer->length) {
        // Unterminated string
        token.type = TOKEN_EOF;
        return token;
    }
    
    token.end = lexer->position;
    lexer->position++; // Skip closing quote
    
    size_t length = token.end - token.start - 1;
    char* string = (char*)malloc(length + 1);
    strncpy(string, lexer->source + token.start + 1, length);
    string[length] = '\0';
    
    token.type = TOKEN_STRING_LITERAL;
    token.value.string = string;
    return token;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    skip_comment(lexer);
    
    if (lexer->position >= lexer->length) {
        Token token;
        token.type = TOKEN_EOF;
        token.start = lexer->position;
        token.end = lexer->position;
        return token;
    }
    
    char c = lexer->source[lexer->position];
    
    if (is_identifier_start(c)) {
        return scan_identifier(lexer);
    }
    
    if (is_digit(c)) {
        return scan_number(lexer);
    }
    
    if (c == '"') {
        return scan_string(lexer);
    }
    
    // Handle single character tokens
    Token token;
    token.start = lexer->position;
    token.end = lexer->position + 1;
    
    switch (c) {
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        case '[': token.type = TOKEN_LBRACKET; break;
        case ']': token.type = TOKEN_RBRACKET; break;
        case ':': token.type = TOKEN_COLON; break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '.': token.type = TOKEN_DOT; break;
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_STAR; break;
        case '/': token.type = TOKEN_SLASH; break;
        case '%': token.type = TOKEN_PERCENT; break;
        case '!': token.type = TOKEN_NOT; break;
        case '_': token.type = TOKEN_UNDERSCORE; break;
        default: token.type = TOKEN_EOF; break;
    }
    
    lexer->position++;
    return token;
}

Token lexer_peek_token(Lexer* lexer) {
    size_t current_position = lexer->position;
    Token token = lexer_next_token(lexer);
    lexer->position = current_position;
    return token;
}

bool lexer_is_at_end(Lexer* lexer) {
    return lexer->position >= lexer->length;
} 