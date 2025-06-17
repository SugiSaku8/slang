#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/type_system.h"
#include "../include/error.h"

// ファイルの内容を読み込む
char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Not enough memory to read '%s'\n", path);
        exit(74);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Error: Could not read file '%s'\n", path);
        exit(74);
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

// コンパイル処理
SlangError compile(const char* source, const char* output_path) {
    // 字句解析
    Lexer* lexer = lexer_create(source);
    if (lexer == NULL) {
        return SLANG_ERROR_INTERNAL;
    }

    SlangError error = lexer_scan(lexer);
    if (error != SLANG_SUCCESS) {
        lexer_destroy(lexer);
        return error;
    }

    // 構文解析
    Parser* parser = parser_create(lexer);
    if (parser == NULL) {
        lexer_destroy(lexer);
        return SLANG_ERROR_INTERNAL;
    }

    ASTNode* ast = NULL;
    error = parser_parse(parser, &ast);
    if (error != SLANG_SUCCESS) {
        parser_destroy(parser);
        lexer_destroy(lexer);
        return error;
    }

    // 型チェック
    error = type_check(ast);
    if (error != SLANG_SUCCESS) {
        ast_destroy_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return error;
    }

    // TODO: コード生成

    // クリーンアップ
    ast_destroy_node(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);

    return SLANG_SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: slangc <source_file>\n");
        return 64;
    }

    const char* source_path = argv[1];
    char* source = read_file(source_path);
    if (source == NULL) {
        return 74;
    }

    // 出力ファイル名を生成
    char* output_path = malloc(strlen(source_path) + 3);
    if (output_path == NULL) {
        free(source);
        return 74;
    }
    strcpy(output_path, source_path);
    strcat(output_path, ".o");

    // コンパイル
    SlangError error = compile(source, output_path);
    free(source);
    free(output_path);

    if (error != SLANG_SUCCESS) {
        return 65;
    }

    return 0;
} 