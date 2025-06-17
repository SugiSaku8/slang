#include "../include/error.h"
#include <stdio.h>
#include <stdlib.h>

// Error codes
#define SLANG_ERROR_SYNTAX 1
#define SLANG_ERROR_TYPE 2
#define SLANG_ERROR_RUNTIME 3

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