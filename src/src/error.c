#include "../include/error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Error type strings
static const char* error_type_strings[] = {
    "Syntax error",
    "Type error",
    "Compilation error",
    "Runtime error",
    "IO error"
};

// Error creation
SlangError* slang_error_new(SlangErrorType type, const char* message) {
    SlangError* error = (SlangError*)malloc(sizeof(SlangError));
    if (!error) return NULL;
    error->type = type;
    error->message = strdup(message);
    if (!error->message) {
        free(error);
        return NULL;
    }
    return error;
}

// Error destruction
void slang_error_free(SlangError* error) {
    if (!error) return;
    free(error->message);
    free(error);
}

// Error to string
const char* slang_error_to_string(const SlangError* error) {
    if (!error) return "Unknown error";
    static char buffer[1024];
    const char* type_str = "Unknown error";
    if (error->type >= 0 && error->type < (sizeof(error_type_strings)/sizeof(error_type_strings[0]))) {
        type_str = error_type_strings[error->type];
    }
    snprintf(buffer, sizeof(buffer), "%s: %s", type_str, error->message);
    return buffer;
} 