#include "../include/error.h"
#include <stdio.h>

void print_error(const SlangError* error) {
    if (!error) return;
    fprintf(stderr, "Error[%d]: %s\n", error->code, error->message);
} 