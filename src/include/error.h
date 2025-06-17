#ifndef SLANG_ERROR_H
#define SLANG_ERROR_H

#include "common.h"
#include "lexer.h"

// エラーの種類
typedef enum {
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_UNDEFINED_VARIABLE,
    ERROR_UNDEFINED_FUNCTION,
    ERROR_DUPLICATE_DECLARATION,
    ERROR_INVALID_OPERATION,
    ERROR_INVALID_ARGUMENT,
    ERROR_INTERNAL
} ErrorType;

// エラーの構造体
typedef struct {
    ErrorType type;
    char* message;
    size_t line;
    size_t column;
    char* file;
} Error;

// エラー処理の関数
Error* error_create(ErrorType type, const char* message, size_t line, size_t column, const char* file);
void error_destroy(Error* error);
void error_report(Error* error);
char* error_to_string(Error* error);

#endif // SLANG_ERROR_H 