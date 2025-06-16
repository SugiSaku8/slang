#include <stdio.h>
#include <stdlib.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/interpreter.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    // Read source file
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", argv[1]);
        return 1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read file content
    char* source = (char*)malloc(file_size + 1);
    if (!source) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    size_t read_size = fread(source, 1, file_size, file);
    source[read_size] = '\0';
    fclose(file);

    // Create lexer
    Lexer* lexer = create_lexer(source);
    if (!lexer) {
        fprintf(stderr, "Error: Failed to create lexer\n");
        free(source);
        return 1;
    }

    // Create parser
    Parser* parser = create_parser(lexer);
    if (!parser) {
        fprintf(stderr, "Error: Failed to create parser\n");
        free_lexer(lexer);
        free(source);
        return 1;
    }

    // Parse program
    ASTNode* program = parse_program(parser);
    if (!program) {
        fprintf(stderr, "Error: Failed to parse program\n");
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return 1;
    }

    // Create interpreter
    Interpreter* interpreter = create_interpreter();
    if (!interpreter) {
        fprintf(stderr, "Error: Failed to create interpreter\n");
        free_ast_node(program);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return 1;
    }

    // Interpret program
    interpret(interpreter, program);

    // Cleanup
    free_interpreter(interpreter);
    free_ast_node(program);
    free_parser(parser);
    free_lexer(lexer);
    free(source);

    return 0;
} 