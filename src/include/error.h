#ifndef ERROR_H
#define ERROR_H

typedef struct {
    int code;
    const char* message;
} SlangError;

void print_error(const SlangError* error);

#endif // ERROR_H 