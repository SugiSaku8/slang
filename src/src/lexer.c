#include "../include/lexer.h"
#include "../include/common.h"
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

// キーワードの定義
typedef struct {
    const char* keyword;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    {"fn", TOKEN_FN},
    {"let", TOKEN_LET},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"struct", TOKEN_STRUCT},
    {"impl", TOKEN_IMPL},
    {"trait", TOKEN_TRAIT},
    {"use", TOKEN_USE},
    {"pub", TOKEN_PUB},
    {"priv", TOKEN_PRIV},
    {NULL, 0}
};

// 字句解析器の作成
Lexer* lexer_create(const char* source) {
    Lexer* lexer = malloc(sizeof(Lexer));
    if (lexer == NULL) return NULL;

    lexer->source = source;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->tokens = vector_create(sizeof(Token));
    
    if (lexer->tokens == NULL) {
        free(lexer);
        return NULL;
    }

    return lexer;
}

// 字句解析器の破棄
void lexer_destroy(Lexer* lexer) {
    if (lexer == NULL) return;
    
    // トークンの文字列を解放
    for (size_t i = 0; i < vector_size(lexer->tokens); i++) {
        Token* token = vector_get(lexer->tokens, i);
        free(token->lexeme);
    }
    
    vector_destroy(lexer->tokens);
    free(lexer);
}

// 現在の文字を取得
static char lexer_current(const Lexer* lexer) {
    return lexer->source[lexer->current];
}

// 次の文字を取得
static char lexer_advance(Lexer* lexer) {
    char c = lexer_current(lexer);
    lexer->current++;
    lexer->column++;
    return c;
}

// 次の文字を確認
static char lexer_peek(const Lexer* lexer) {
    return lexer->source[lexer->current];
}

// 次の次の文字を確認
static char lexer_peek_next(const Lexer* lexer) {
    if (lexer->current + 1 >= strlen(lexer->source)) return '\0';
    return lexer->source[lexer->current + 1];
}

// 現在のトークンの文字列を取得
static char* lexer_get_lexeme(const Lexer* lexer) {
    size_t length = lexer->current - lexer->start;
    char* lexeme = malloc(length + 1);
    if (lexeme == NULL) return NULL;
    
    strncpy(lexeme, lexer->source + lexer->start, length);
    lexeme[length] = '\0';
    return lexeme;
}

// トークンの追加
static bool lexer_add_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.lexeme = lexer_get_lexeme(lexer);
    token.line = lexer->line;
    token.column = lexer->column - (lexer->current - lexer->start);
    
    return vector_push(lexer->tokens, &token);
}

// 識別子の解析
static void lexer_identifier(Lexer* lexer) {
    while (isalnum(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
        lexer_advance(lexer);
    }

    // キーワードの確認
    char* lexeme = lexer_get_lexeme(lexer);
    TokenType type = TOKEN_IDENTIFIER;
    
    for (const Keyword* kw = keywords; kw->keyword != NULL; kw++) {
        if (strcmp(lexeme, kw->keyword) == 0) {
            type = kw->type;
            break;
        }
    }
    
    free(lexeme);
    lexer_add_token(lexer, type);
}

// 数値の解析
static void lexer_number(Lexer* lexer) {
    while (isdigit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }

    // 小数点の確認
    if (lexer_peek(lexer) == '.' && isdigit(lexer_peek_next(lexer))) {
        lexer_advance(lexer); // 小数点を消費
        
        while (isdigit(lexer_peek(lexer))) {
            lexer_advance(lexer);
        }
        
        lexer_add_token(lexer, TOKEN_FLOAT);
    } else {
        lexer_add_token(lexer, TOKEN_INTEGER);
    }
}

// 文字列の解析
static void lexer_string(Lexer* lexer) {
    while (lexer_peek(lexer) != '"' && lexer_peek(lexer) != '\0') {
        if (lexer_peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
        lexer_advance(lexer);
    }

    if (lexer_peek(lexer) == '\0') {
        // エラー: 文字列が終了していない
        lexer_add_token(lexer, TOKEN_ERROR);
        return;
    }

    // 閉じ引用符を消費
    lexer_advance(lexer);
    
    // 文字列リテラルをトークンとして追加
    lexer_add_token(lexer, TOKEN_STRING);
}

// コメントの解析
static void lexer_comment(Lexer* lexer) {
    while (lexer_peek(lexer) != '\n' && lexer_peek(lexer) != '\0') {
        lexer_advance(lexer);
    }
}

// トークンのスキャン
SlangError lexer_scan(Lexer* lexer) {
    while (lexer_peek(lexer) != '\0') {
        lexer->start = lexer->current;
        char c = lexer_advance(lexer);

        switch (c) {
            case '(': lexer_add_token(lexer, TOKEN_LPAREN); break;
            case ')': lexer_add_token(lexer, TOKEN_RPAREN); break;
            case '{': lexer_add_token(lexer, TOKEN_LBRACE); break;
            case '}': lexer_add_token(lexer, TOKEN_RBRACE); break;
            case ';': lexer_add_token(lexer, TOKEN_SEMICOLON); break;
            case ',': lexer_add_token(lexer, TOKEN_COMMA); break;
            case '.': lexer_add_token(lexer, TOKEN_DOT); break;
            case '-': 
                if (lexer_peek(lexer) == '>') {
                    lexer_advance(lexer);
                    lexer_add_token(lexer, TOKEN_ARROW);
                } else {
                    lexer_add_token(lexer, TOKEN_MINUS);
                }
                break;
            case '+': lexer_add_token(lexer, TOKEN_PLUS); break;
            case '*': lexer_add_token(lexer, TOKEN_STAR); break;
            case '/':
                if (lexer_peek(lexer) == '/') {
                    lexer_comment(lexer);
                } else {
                    lexer_add_token(lexer, TOKEN_SLASH);
                }
                break;
            case '%': lexer_add_token(lexer, TOKEN_PERCENT); break;
            case '=':
                if (lexer_peek(lexer) == '=') {
                    lexer_advance(lexer);
                    lexer_add_token(lexer, TOKEN_EQ);
                } else {
                    lexer_add_token(lexer, TOKEN_EQUAL);
                }
                break;
            case '!':
                if (lexer_peek(lexer) == '=') {
                    lexer_advance(lexer);
                    lexer_add_token(lexer, TOKEN_NEQ);
                }
                break;
            case '<':
                if (lexer_peek(lexer) == '=') {
                    lexer_advance(lexer);
                    lexer_add_token(lexer, TOKEN_LE);
                } else {
                    lexer_add_token(lexer, TOKEN_LT);
                }
                break;
            case '>':
                if (lexer_peek(lexer) == '=') {
                    lexer_advance(lexer);
                    lexer_add_token(lexer, TOKEN_GE);
                } else {
                    lexer_add_token(lexer, TOKEN_GT);
                }
                break;
            case '"': lexer_string(lexer); break;
            case ' ':
            case '\r':
            case '\t':
                // 空白は無視
                break;
            case '\n':
                lexer->line++;
                lexer->column = 1;
                break;
            default:
                if (isdigit(c)) {
                    lexer_number(lexer);
                } else if (isalpha(c) || c == '_') {
                    lexer_identifier(lexer);
                } else {
                    // エラー: 不正な文字
                    lexer_add_token(lexer, TOKEN_ERROR);
                }
                break;
        }
    }

    // EOFトークンを追加
    lexer_add_token(lexer, TOKEN_EOF);
    return SLANG_SUCCESS;
}

// 次のトークンを取得
Token* lexer_next_token(Lexer* lexer) {
    static size_t current = 0;
    if (current >= vector_size(lexer->tokens)) return NULL;
    return vector_get(lexer->tokens, current++);
}

// 次のトークンを確認
Token* lexer_peek_token(Lexer* lexer) {
    static size_t current = 0;
    if (current >= vector_size(lexer->tokens)) return NULL;
    return vector_get(lexer->tokens, current);
} 