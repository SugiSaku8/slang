#include "../include/error.h"
#include "../include/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Error codes
#define SLANG_ERROR_SYNTAX 1
#define SLANG_ERROR_TYPE 2
#define SLANG_ERROR_RUNTIME 3

// エラーの作成
Error* error_create(ErrorType type, const char* message, size_t line, size_t column, const char* file) {
    Error* error = malloc(sizeof(Error));
    if (error == NULL) return NULL;

    error->type = type;
    error->message = strdup(message);
    error->line = line;
    error->column = column;
    error->file = file ? strdup(file) : NULL;

    return error;
}

// エラーの破棄
void error_destroy(Error* error) {
    if (error == NULL) return;
    free(error->message);
    free(error->file);
    free(error);
}

// エラーの報告
void error_report(Error* error) {
    if (error == NULL) return;

    // エラーの種類に応じたメッセージ
    const char* type_str;
    switch (error->type) {
        case ERROR_SYNTAX:
            type_str = "Syntax Error";
            break;
        case ERROR_TYPE:
            type_str = "Type Error";
            break;
        case ERROR_UNDEFINED_VARIABLE:
            type_str = "Undefined Variable Error";
            break;
        case ERROR_UNDEFINED_FUNCTION:
            type_str = "Undefined Function Error";
            break;
        case ERROR_DUPLICATE_DECLARATION:
            type_str = "Duplicate Declaration Error";
            break;
        case ERROR_INVALID_OPERATION:
            type_str = "Invalid Operation Error";
            break;
        case ERROR_INVALID_ARGUMENT:
            type_str = "Invalid Argument Error";
            break;
        case ERROR_INTERNAL:
            type_str = "Internal Error";
            break;
        default:
            type_str = "Unknown Error";
            break;
    }

    // エラーメッセージの出力
    if (error->file) {
        fprintf(stderr, "%s:%zu:%zu: %s: %s\n",
                error->file, error->line, error->column,
                type_str, error->message);
    } else {
        fprintf(stderr, "%zu:%zu: %s: %s\n",
                error->line, error->column,
                type_str, error->message);
    }
}

// エラーの文字列表現を取得
char* error_to_string(Error* error) {
    if (error == NULL) return strdup("Unknown error");

    // エラーの種類に応じたメッセージ
    const char* type_str;
    switch (error->type) {
        case ERROR_SYNTAX:
            type_str = "Syntax Error";
            break;
        case ERROR_TYPE:
            type_str = "Type Error";
            break;
        case ERROR_UNDEFINED_VARIABLE:
            type_str = "Undefined Variable Error";
            break;
        case ERROR_UNDEFINED_FUNCTION:
            type_str = "Undefined Function Error";
            break;
        case ERROR_DUPLICATE_DECLARATION:
            type_str = "Duplicate Declaration Error";
            break;
        case ERROR_INVALID_OPERATION:
            type_str = "Invalid Operation Error";
            break;
        case ERROR_INVALID_ARGUMENT:
            type_str = "Invalid Argument Error";
            break;
        case ERROR_INTERNAL:
            type_str = "Internal Error";
            break;
        default:
            type_str = "Unknown Error";
            break;
    }

    // エラーメッセージの文字列を作成
    char* result;
    if (error->file) {
        result = malloc(strlen(error->file) + strlen(type_str) + strlen(error->message) + 50);
        sprintf(result, "%s:%zu:%zu: %s: %s",
                error->file, error->line, error->column,
                type_str, error->message);
    } else {
        result = malloc(strlen(type_str) + strlen(error->message) + 50);
        sprintf(result, "%zu:%zu: %s: %s",
                error->line, error->column,
                type_str, error->message);
    }

    return result;
}

SlangError* slang_error_new(int code, const char* message) {
    SlangError* error = (SlangError*)malloc(sizeof(SlangError));
    if (error == NULL) {
        return NULL;
    }
    error->code = code;
    error->message = message;
    return error;
}

void print_error(const SlangError* error) {
    if (!error) return;
    fprintf(stderr, "Error[%d]: %s\n", error->code, error->message);
} 