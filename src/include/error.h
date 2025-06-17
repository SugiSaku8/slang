#ifndef ERROR_H
#define ERROR_H

typedef struct {
    int code;
    const char* message;
} SlangError;

// Error codes
#define SLANG_ERROR_SYNTAX 1
#define SLANG_ERROR_TYPE 2
#define SLANG_ERROR_RUNTIME 3

SlangError* slang_error_new(int code, const char* message);
void print_error(const SlangError* error);

#endif // ERROR_H 