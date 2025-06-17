#include "../include/codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// コード生成コンテキストの作成
CodeGenContext* codegen_create(const char* output_path) {
    CodeGenContext* context = malloc(sizeof(CodeGenContext));
    if (context == NULL) return NULL;

    context->output_path = output_path;
    context->output_file = fopen(output_path, "w");
    if (context->output_file == NULL) {
        free(context);
        return NULL;
    }

    context->string_literals = vector_create(sizeof(char*));
    context->global_variables = vector_create(sizeof(char*));
    context->functions = vector_create(sizeof(char*));
    context->label_counter = 0;

    if (context->string_literals == NULL ||
        context->global_variables == NULL ||
        context->functions == NULL) {
        codegen_destroy(context);
        return NULL;
    }

    return context;
}

// コード生成コンテキストの破棄
void codegen_destroy(CodeGenContext* context) {
    if (context == NULL) return;

    if (context->output_file) {
        fclose(context->output_file);
    }

    // 文字列リテラルの解放
    for (size_t i = 0; i < vector_size(context->string_literals); i++) {
        char** str = vector_get(context->string_literals, i);
        free(*str);
    }
    vector_destroy(context->string_literals);

    // グローバル変数の解放
    for (size_t i = 0; i < vector_size(context->global_variables); i++) {
        char** var = vector_get(context->global_variables, i);
        free(*var);
    }
    vector_destroy(context->global_variables);

    // 関数名の解放
    for (size_t i = 0; i < vector_size(context->functions); i++) {
        char** func = vector_get(context->functions, i);
        free(*func);
    }
    vector_destroy(context->functions);

    free(context);
}

// 新しいラベルの生成
static char* codegen_new_label(CodeGenContext* context) {
    char* label = malloc(32);
    if (label == NULL) return NULL;
    sprintf(label, "L%zu", context->label_counter++);
    return label;
}

// プロローグの生成
void codegen_emit_prologue(CodeGenContext* context) {
    fprintf(context->output_file, ".section .text\n");
    fprintf(context->output_file, ".global main\n");
    fprintf(context->output_file, "\n");
}

// エピローグの生成
void codegen_emit_epilogue(CodeGenContext* context) {
    fprintf(context->output_file, "\n.section .data\n");
    
    // 文字列リテラルの出力
    for (size_t i = 0; i < vector_size(context->string_literals); i++) {
        char** str = vector_get(context->string_literals, i);
        fprintf(context->output_file, "str_%zu: .asciz \"%s\"\n", i, *str);
    }
    
    // グローバル変数の出力
    for (size_t i = 0; i < vector_size(context->global_variables); i++) {
        char** var = vector_get(context->global_variables, i);
        fprintf(context->output_file, "%s: .quad 0\n", *var);
    }
}

// 関数の生成
void codegen_emit_function(CodeGenContext* context, ASTNode* node) {
    if (node == NULL || node->type != NODE_FUNCTION_DECL) return;
    if (node->as.function.name == NULL) return;

    fprintf(context->output_file, "\n%s:\n", node->as.function.name);
    fprintf(context->output_file, "    push rbp\n");
    fprintf(context->output_file, "    mov rbp, rsp\n");

    // ローカル変数のためのスタック領域の確保
    if (node->as.function.local_size > 0) {
        fprintf(context->output_file, "    sub rsp, %zu\n", node->as.function.local_size);
    }

    // パラメータの処理
    if (node->as.function.parameters) {
        size_t param_count = vector_size(node->as.function.parameters);
        for (size_t i = 0; i < param_count; i++) {
            ASTNode** param = vector_get(node->as.function.parameters, i);
            if (param == NULL || *param == NULL) continue;
            
            ASTNode* param_node = *param;
            if (param_node->type != NODE_IDENTIFIER) continue;
            
            // パラメータをスタックから適切な位置に移動
            // System V AMD64 ABIに従って、最初の6つのパラメータはレジスタで渡される
            if (i < 6) {
                const char* regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                fprintf(context->output_file, "    mov [rbp - %zu], %s\n",
                        param_node->as.identifier.offset, regs[i]);
            } else {
                // 7番目以降のパラメータはスタックに積まれている
                fprintf(context->output_file, "    mov rax, [rbp + %zu]\n",
                        16 + (i - 6) * 8);  // 16はrbpとreturn addressの分
                fprintf(context->output_file, "    mov [rbp - %zu], rax\n",
                        param_node->as.identifier.offset);
            }
        }
    }

    // 関数本体の生成
    if (node->as.function.body) {
        codegen_emit_statement(context, node->as.function.body);
    }

    // スタックフレームのクリーンアップ
    fprintf(context->output_file, "    mov rsp, rbp\n");
    fprintf(context->output_file, "    pop rbp\n");
    fprintf(context->output_file, "    ret\n");
}

// 文の生成
void codegen_emit_statement(CodeGenContext* context, ASTNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_BLOCK:
            if (node->as.block.statements) {
                for (size_t i = 0; i < vector_size(node->as.block.statements); i++) {
                    ASTNode** stmt = vector_get(node->as.block.statements, i);
                    if (stmt == NULL || *stmt == NULL) continue;
                    codegen_emit_statement(context, *stmt);
                }
            }
            break;

        case NODE_IF:
            if (node->as.if_stmt.condition == NULL) return;
            
            // 条件式の評価
            codegen_emit_expression(context, node->as.if_stmt.condition);
            
            // 条件分岐
            char* else_label = codegen_new_label(context);
            char* end_label = codegen_new_label(context);
            if (else_label == NULL || end_label == NULL) return;
            
            fprintf(context->output_file, "    cmp rax, 0\n");
            fprintf(context->output_file, "    je %s\n", else_label);
            
            // then節の生成
            if (node->as.if_stmt.then_branch) {
                codegen_emit_statement(context, node->as.if_stmt.then_branch);
            }
            fprintf(context->output_file, "    jmp %s\n", end_label);
            
            // else節の生成
            fprintf(context->output_file, "%s:\n", else_label);
            if (node->as.if_stmt.else_branch) {
                codegen_emit_statement(context, node->as.if_stmt.else_branch);
            }
            
            fprintf(context->output_file, "%s:\n", end_label);
            free(else_label);
            free(end_label);
            break;

        case NODE_WHILE:
            if (node->as.while_stmt.condition == NULL) return;
            
            // ラベルの生成
            char* start_label = codegen_new_label(context);
            char* exit_label = codegen_new_label(context);
            if (start_label == NULL || exit_label == NULL) return;
            
            // ループ開始
            fprintf(context->output_file, "%s:\n", start_label);
            
            // 条件式の評価
            codegen_emit_expression(context, node->as.while_stmt.condition);
            fprintf(context->output_file, "    cmp rax, 0\n");
            fprintf(context->output_file, "    je %s\n", exit_label);
            
            // ループ本体の生成
            if (node->as.while_stmt.body) {
                codegen_emit_statement(context, node->as.while_stmt.body);
            }
            fprintf(context->output_file, "    jmp %s\n", start_label);
            
            // ループ終了
            fprintf(context->output_file, "%s:\n", exit_label);
            free(start_label);
            free(exit_label);
            break;

        case NODE_RETURN:
            if (node->as.return_stmt.value) {
                codegen_emit_expression(context, node->as.return_stmt.value);
            }
            fprintf(context->output_file, "    mov rsp, rbp\n");
            fprintf(context->output_file, "    pop rbp\n");
            fprintf(context->output_file, "    ret\n");
            break;

        default:
            // 式文として処理
            codegen_emit_expression(context, node);
            break;
    }
}

// 式の生成
void codegen_emit_expression(CodeGenContext* context, ASTNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_LITERAL:
            codegen_emit_literal(context, node);
            break;

        case NODE_BINARY:
            codegen_emit_binary(context, node);
            break;

        case NODE_UNARY:
            codegen_emit_unary(context, node);
            break;

        case NODE_CALL:
            codegen_emit_call(context, node);
            break;

        case NODE_IDENTIFIER:
            codegen_emit_variable(context, node);
            break;

        default:
            break;
    }
}

// リテラルの生成
void codegen_emit_literal(CodeGenContext* context, ASTNode* node) {
    switch (node->as.literal.type) {
        case LITERAL_INTEGER:
            fprintf(context->output_file, "    mov rax, %ld\n", node->as.literal.value.integer);
            break;

        case LITERAL_FLOAT:
            // 浮動小数点数をメモリに配置
            fprintf(context->output_file, "    mov rax, %ld\n", 
                    *(int64_t*)&node->as.literal.value.float_val);
            fprintf(context->output_file, "    movq xmm0, rax\n");
            break;

        case LITERAL_STRING:
            // 文字列リテラルをデータセクションに追加
            vector_push(context->string_literals, &node->as.literal.value.string);
            fprintf(context->output_file, "    lea rax, [rel str_%zu]\n", 
                    vector_size(context->string_literals) - 1);
            break;

        case LITERAL_BOOLEAN:
            fprintf(context->output_file, "    mov rax, %d\n", 
                    node->as.literal.value.boolean ? 1 : 0);
            break;

        case LITERAL_NULL:
            fprintf(context->output_file, "    xor rax, rax\n");
            break;
    }
}

// 二項演算の生成
void codegen_emit_binary(CodeGenContext* context, ASTNode* node) {
    // 右辺の評価
    codegen_emit_expression(context, node->as.binary.right);
    fprintf(context->output_file, "    push rax\n");
    
    // 左辺の評価
    codegen_emit_expression(context, node->as.binary.left);
    fprintf(context->output_file, "    pop rbx\n");
    
    // 演算子に応じた処理
    switch (node->as.binary.operator) {
        case TOKEN_PLUS:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    addsd xmm0, xmm1\n");
                fprintf(context->output_file, "    movq rax, xmm0\n");
            } else {
                fprintf(context->output_file, "    add rax, rbx\n");
            }
            break;
            
        case TOKEN_MINUS:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    subsd xmm0, xmm1\n");
                fprintf(context->output_file, "    movq rax, xmm0\n");
            } else {
                fprintf(context->output_file, "    sub rax, rbx\n");
            }
            break;
            
        case TOKEN_STAR:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    mulsd xmm0, xmm1\n");
                fprintf(context->output_file, "    movq rax, xmm0\n");
            } else {
                fprintf(context->output_file, "    imul rax, rbx\n");
            }
            break;
            
        case TOKEN_SLASH:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    divsd xmm0, xmm1\n");
                fprintf(context->output_file, "    movq rax, xmm0\n");
            } else {
                fprintf(context->output_file, "    cqo\n");
                fprintf(context->output_file, "    idiv rbx\n");
            }
            break;
            
        case TOKEN_EQ:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    comisd xmm0, xmm1\n");
                fprintf(context->output_file, "    sete al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            } else {
                fprintf(context->output_file, "    cmp rax, rbx\n");
                fprintf(context->output_file, "    sete al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            }
            break;
            
        case TOKEN_NEQ:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    comisd xmm0, xmm1\n");
                fprintf(context->output_file, "    setne al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            } else {
                fprintf(context->output_file, "    cmp rax, rbx\n");
                fprintf(context->output_file, "    setne al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            }
            break;
            
        case TOKEN_LT:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    comisd xmm0, xmm1\n");
                fprintf(context->output_file, "    setb al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            } else {
                fprintf(context->output_file, "    cmp rax, rbx\n");
                fprintf(context->output_file, "    setl al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            }
            break;
            
        case TOKEN_GT:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    comisd xmm0, xmm1\n");
                fprintf(context->output_file, "    seta al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            } else {
                fprintf(context->output_file, "    cmp rax, rbx\n");
                fprintf(context->output_file, "    setg al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            }
            break;
            
        case TOKEN_LE:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    comisd xmm0, xmm1\n");
                fprintf(context->output_file, "    setbe al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            } else {
                fprintf(context->output_file, "    cmp rax, rbx\n");
                fprintf(context->output_file, "    setle al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            }
            break;
            
        case TOKEN_GE:
            if (node->as.binary.type == TYPE_FLOAT) {
                fprintf(context->output_file, "    movq xmm0, rax\n");
                fprintf(context->output_file, "    movq xmm1, rbx\n");
                fprintf(context->output_file, "    comisd xmm0, xmm1\n");
                fprintf(context->output_file, "    setae al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            } else {
                fprintf(context->output_file, "    cmp rax, rbx\n");
                fprintf(context->output_file, "    setge al\n");
                fprintf(context->output_file, "    movzx rax, al\n");
            }
            break;
            
        default:
            break;
    }
}

// 単項演算の生成
void codegen_emit_unary(CodeGenContext* context, ASTNode* node) {
    // オペランドの評価
    codegen_emit_expression(context, node->as.unary.operand);
    
    // 演算子に応じた処理
    switch (node->as.unary.operator) {
        case TOKEN_MINUS:
            fprintf(context->output_file, "    neg rax\n");
            break;
            
        case TOKEN_BANG:
            fprintf(context->output_file, "    test rax, rax\n");
            fprintf(context->output_file, "    setz al\n");
            fprintf(context->output_file, "    movzx rax, al\n");
            break;
            
        default:
            break;
    }
}

// 関数呼び出しの生成
void codegen_emit_call(CodeGenContext* context, ASTNode* node) {
    // 引数の評価（右から左へ）
    if (node->as.call.arguments) {
        for (size_t i = vector_size(node->as.call.arguments); i > 0; i--) {
            ASTNode** arg = vector_get(node->as.call.arguments, i - 1);
            codegen_emit_expression(context, *arg);
            fprintf(context->output_file, "    push rax\n");
        }
    }
    
    // 関数の呼び出し
    codegen_emit_expression(context, node->as.call.callee);
    fprintf(context->output_file, "    call rax\n");
    
    // スタックの調整
    if (node->as.call.arguments) {
        fprintf(context->output_file, "    add rsp, %zu\n", 
                vector_size(node->as.call.arguments) * 8);
    }
}

// 変数の生成
void codegen_emit_variable(CodeGenContext* context, ASTNode* node) {
    if (node->type != NODE_IDENTIFIER) return;

    // ローカル変数の場合
    if (node->as.identifier.is_local) {
        fprintf(context->output_file, "    mov rax, [rbp - %zu]\n", 
                node->as.identifier.offset);
    }
    // グローバル変数の場合
    else {
        fprintf(context->output_file, "    mov rax, [rel %s]\n", 
                node->as.identifier.name);
    }
}

// コード生成のメイン関数
SlangError codegen_generate(CodeGenContext* context, ASTNode* ast) {
    if (context == NULL || ast == NULL) return SLANG_ERROR_INTERNAL;
    if (context->output_file == NULL) return SLANG_ERROR_INTERNAL;

    // プロローグの生成
    codegen_emit_prologue(context);

    // プログラムの生成
    if (ast->type == NODE_PROGRAM && ast->as.program.declarations) {
        for (size_t i = 0; i < vector_size(ast->as.program.declarations); i++) {
            ASTNode** decl = vector_get(ast->as.program.declarations, i);
            if ((*decl)->type == NODE_FUNCTION_DECL) {
                codegen_emit_function(context, *decl);
            }
        }
    } else {
        return SLANG_ERROR_INVALID_AST;
    }

    // エピローグの生成
    codegen_emit_epilogue(context);

    // 出力ファイルのフラッシュ
    if (fflush(context->output_file) != 0) {
        return SLANG_ERROR_IO;
    }

    return SLANG_SUCCESS;
} 