#include "error.h"
#include <stdlib.h>
#include <string.h>

SlangError* slang_error_new(SlangErrorType type, const char* message) {
    SlangError* error = (SlangError*)malloc(sizeof(SlangError));
    if (error == NULL) {
        return NULL;
    }

    error->type = type;
    error->message = strdup(message);
    if (error->message == NULL) {
        free(error);
        return NULL;
    }

    return error;
}

void slang_error_free(SlangError* error) {
    if (error != NULL) {
        free(error->message);
        free(error);
    }
}

const char* slang_error_to_string(const SlangError* error) {
    if (error == NULL) {
        return "No error";
    }

    static char buffer[1024];
    const char* type_str;

    switch (error->type) {
        case SLANG_ERROR_SYNTAX:
            type_str = "Syntax error";
            break;
        case SLANG_ERROR_TYPE:
            type_str = "Type error";
            break;
        case SLANG_ERROR_COMPILATION:
            type_str = "Compilation error";
            break;
        case SLANG_ERROR_RUNTIME:
            type_str = "Runtime error";
            break;
        case SLANG_ERROR_IO:
            type_str = "IO error";
            break;
        default:
            type_str = "Unknown error";
    }

    snprintf(buffer, sizeof(buffer), "%s: %s", type_str, error->message);
    return buffer;
} 