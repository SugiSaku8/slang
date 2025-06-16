#ifndef SLANG_ERROR_H
#define SLANG_ERROR_H

#include <stdbool.h>

typedef enum {
    SLANG_ERROR_SYNTAX,
    SLANG_ERROR_TYPE,
    SLANG_ERROR_COMPILATION,
    SLANG_ERROR_RUNTIME,
    SLANG_ERROR_IO
} SlangErrorType;

typedef struct {
    SlangErrorType type;
    char* message;
} SlangError;

// Error handling functions
SlangError* slang_error_new(SlangErrorType type, const char* message);
void slang_error_free(SlangError* error);
const char* slang_error_to_string(const SlangError* error);

// Error checking macro
#define SLANG_CHECK_ERROR(expr) do { \
    SlangError* _err = (expr); \
    if (_err != NULL) { \
        return _err; \
    } \
} while(0)

#endif // SLANG_ERROR_H 