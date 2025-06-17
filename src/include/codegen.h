#ifndef SLANG_CODEGEN_H
#define SLANG_CODEGEN_H

#include "ast.h"
#include "type_system.h"
#include "common.h"

// コード生成のコンテキスト
typedef struct {
    const char* output_path;
    FILE* output_file;
    Vector* string_literals;
    Vector* global_variables;
    Vector* functions;
    size_t label_counter;
} CodeGenContext;

// コード生成の関数
CodeGenContext* codegen_create(const char* output_path);
void codegen_destroy(CodeGenContext* context);
SlangError codegen_generate(CodeGenContext* context, ASTNode* ast);

// アセンブリ生成の関数
void codegen_emit_prologue(CodeGenContext* context);
void codegen_emit_epilogue(CodeGenContext* context);
void codegen_emit_function(CodeGenContext* context, ASTNode* node);
void codegen_emit_statement(CodeGenContext* context, ASTNode* node);
void codegen_emit_expression(CodeGenContext* context, ASTNode* node);
void codegen_emit_literal(CodeGenContext* context, ASTNode* node);
void codegen_emit_binary(CodeGenContext* context, ASTNode* node);
void codegen_emit_unary(CodeGenContext* context, ASTNode* node);
void codegen_emit_call(CodeGenContext* context, ASTNode* node);
void codegen_emit_variable(CodeGenContext* context, ASTNode* node);

#endif // SLANG_CODEGEN_H 