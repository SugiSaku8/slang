#include "../include/ast.h"
#include "../include/common.h"
#include <stdlib.h>
#include <string.h>

// ASTノードの作成
ASTNode* ast_create_node(NodeType type, size_t line, size_t column) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (node == NULL) return NULL;

    node->type = type;
    node->line = line;
    node->column = column;
    node->next = NULL;
    return node;
}

// ASTノードの破棄
void ast_destroy_node(ASTNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_PROGRAM:
            if (node->as.program.declarations) {
                for (size_t i = 0; i < vector_size(node->as.program.declarations); i++) {
                    ASTNode** decl = vector_get(node->as.program.declarations, i);
                    ast_destroy_node(*decl);
                }
                vector_destroy(node->as.program.declarations);
            }
            break;

        case NODE_FUNCTION_DECL:
            free(node->as.function.name);
            if (node->as.function.parameters) {
                for (size_t i = 0; i < vector_size(node->as.function.parameters); i++) {
                    ASTNode** param = vector_get(node->as.function.parameters, i);
                    ast_destroy_node(*param);
                }
                vector_destroy(node->as.function.parameters);
            }
            ast_destroy_node(node->as.function.body);
            ast_destroy_node(node->as.function.return_type);
            break;

        case NODE_VARIABLE_DECL:
            free(node->as.variable.name);
            ast_destroy_node(node->as.variable.initializer);
            ast_destroy_node(node->as.variable.type);
            break;

        case NODE_STRUCT_DECL:
            free(node->as.structure.name);
            if (node->as.structure.fields) {
                for (size_t i = 0; i < vector_size(node->as.structure.fields); i++) {
                    ASTNode** field = vector_get(node->as.structure.fields, i);
                    ast_destroy_node(*field);
                }
                vector_destroy(node->as.structure.fields);
            }
            break;

        case NODE_IMPL_DECL:
            free(node->as.implementation.trait_name);
            free(node->as.implementation.type_name);
            if (node->as.implementation.methods) {
                for (size_t i = 0; i < vector_size(node->as.implementation.methods); i++) {
                    ASTNode** method = vector_get(node->as.implementation.methods, i);
                    ast_destroy_node(*method);
                }
                vector_destroy(node->as.implementation.methods);
            }
            break;

        case NODE_TRAIT_DECL:
            free(node->as.trait.name);
            if (node->as.trait.methods) {
                for (size_t i = 0; i < vector_size(node->as.trait.methods); i++) {
                    ASTNode** method = vector_get(node->as.trait.methods, i);
                    ast_destroy_node(*method);
                }
                vector_destroy(node->as.trait.methods);
            }
            break;

        case NODE_BLOCK:
            if (node->as.block.statements) {
                for (size_t i = 0; i < vector_size(node->as.block.statements); i++) {
                    ASTNode** stmt = vector_get(node->as.block.statements, i);
                    ast_destroy_node(*stmt);
                }
                vector_destroy(node->as.block.statements);
            }
            break;

        case NODE_IF:
            ast_destroy_node(node->as.if_stmt.condition);
            ast_destroy_node(node->as.if_stmt.then_branch);
            ast_destroy_node(node->as.if_stmt.else_branch);
            break;

        case NODE_WHILE:
            ast_destroy_node(node->as.while_stmt.condition);
            ast_destroy_node(node->as.while_stmt.body);
            break;

        case NODE_FOR:
            ast_destroy_node(node->as.for_stmt.initializer);
            ast_destroy_node(node->as.for_stmt.condition);
            ast_destroy_node(node->as.for_stmt.increment);
            ast_destroy_node(node->as.for_stmt.body);
            break;

        case NODE_BINARY:
            ast_destroy_node(node->as.binary.left);
            ast_destroy_node(node->as.binary.right);
            break;

        case NODE_UNARY:
            ast_destroy_node(node->as.unary.operand);
            break;

        case NODE_CALL:
            ast_destroy_node(node->as.call.callee);
            if (node->as.call.arguments) {
                for (size_t i = 0; i < vector_size(node->as.call.arguments); i++) {
                    ASTNode** arg = vector_get(node->as.call.arguments, i);
                    ast_destroy_node(*arg);
                }
                vector_destroy(node->as.call.arguments);
            }
            break;

        case NODE_MEMBER:
            ast_destroy_node(node->as.member.object);
            free(node->as.member.member);
            break;

        case NODE_ARRAY_ACCESS:
            ast_destroy_node(node->as.array_access.array);
            ast_destroy_node(node->as.array_access.index);
            break;

        case NODE_LITERAL:
            if (node->as.literal.type == LITERAL_STRING) {
                free(node->as.literal.value.string);
            }
            break;

        case NODE_IDENTIFIER:
            free(node->as.identifier.name);
            break;

        default:
            break;
    }

    free(node);
}

// プログラムノードの作成
ASTNode* ast_create_program(Vector* declarations) {
    ASTNode* node = ast_create_node(NODE_PROGRAM, 1, 1);
    if (node == NULL) return NULL;

    node->as.program.declarations = declarations;
    return node;
}

// 関数宣言ノードの作成
ASTNode* ast_create_function_decl(char* name, Vector* parameters, ASTNode* body, ASTNode* return_type) {
    ASTNode* node = ast_create_node(NODE_FUNCTION_DECL, 1, 1);
    if (node == NULL) return NULL;

    node->as.function.name = strdup(name);
    node->as.function.parameters = parameters;
    node->as.function.body = body;
    node->as.function.return_type = return_type;
    return node;
}

// 変数宣言ノードの作成
ASTNode* ast_create_variable_decl(char* name, ASTNode* initializer, ASTNode* type) {
    ASTNode* node = ast_create_node(NODE_VARIABLE_DECL, 1, 1);
    if (node == NULL) return NULL;

    node->as.variable.name = strdup(name);
    node->as.variable.initializer = initializer;
    node->as.variable.type = type;
    return node;
}

// 二項演算ノードの作成
ASTNode* ast_create_binary(TokenType operator, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_create_node(NODE_BINARY, left->line, left->column);
    if (node == NULL) return NULL;

    node->as.binary.operator = operator;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

// 単項演算ノードの作成
ASTNode* ast_create_unary(TokenType operator, ASTNode* operand) {
    ASTNode* node = ast_create_node(NODE_UNARY, operand->line, operand->column);
    if (node == NULL) return NULL;

    node->as.unary.operator = operator;
    node->as.unary.operand = operand;
    return node;
}

// リテラルノードの作成
ASTNode* ast_create_literal(LiteralType type, LiteralValue value) {
    ASTNode* node = ast_create_node(NODE_LITERAL, 1, 1);
    if (node == NULL) return NULL;

    node->as.literal.type = type;
    node->as.literal.value = value;
    return node;
}

// 識別子ノードの作成
ASTNode* ast_create_identifier(char* name) {
    ASTNode* node = ast_create_node(NODE_IDENTIFIER, 1, 1);
    if (node == NULL) return NULL;

    node->as.identifier.name = strdup(name);
    return node;
}

ASTNode* create_variable_node(const char* name, Type* type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VARIABLE;
    node->data.variable.name = strdup(name);
    node->data.variable.type = type;
    return node;
}

ASTNode* create_function_node(const char* name, Type* return_type, Variable** parameters, size_t parameter_count, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION;
    node->data.function.name = strdup(name);
    node->data.function.return_type = return_type;
    node->data.function.parameters = parameters;
    node->data.function.parameter_count = parameter_count;
    node->data.function.body = body;
    return node;
}

ASTNode* create_let_statement_node(const char* name, Type* type, ASTNode* initializer) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_LET_STATEMENT;
    node->data.let_statement.name = strdup(name);
    node->data.let_statement.type = type;
    node->data.let_statement.initializer = initializer;
    return node;
}

ASTNode* create_if_statement_node(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IF_STATEMENT;
    node->data.if_statement.condition = condition;
    node->data.if_statement.then_branch = then_branch;
    node->data.if_statement.else_branch = else_branch;
    return node;
}

ASTNode* create_while_statement_node(ASTNode* condition, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_WHILE_STATEMENT;
    node->data.while_statement.condition = condition;
    node->data.while_statement.body = body;
    return node;
}

ASTNode* create_call_expression_node(ASTNode* callee, ASTNode** arguments, size_t argument_count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_CALL_EXPRESSION;
    node->data.call_expression.callee = callee;
    node->data.call_expression.arguments = arguments;
    node->data.call_expression.argument_count = argument_count;
    return node;
}

ASTNode* create_function_call_node(const char* name, ASTNode** arguments, size_t argument_count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->data.function_call.name = strdup(name);
    node->data.function_call.arguments = arguments;
    node->data.function_call.argument_count = argument_count;
    return node;
}

ASTNode* create_assignment_node(const char* name, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGNMENT;
    node->data.assignment.name = strdup(name);
    node->data.assignment.value = value;
    return node;
}

ASTNode* create_variable_reference_node(const char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VARIABLE_REFERENCE;
    node->data.variable_reference.name = strdup(name);
    return node;
}

ASTNode* create_integer_literal_node(int64_t value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_INTEGER_LITERAL;
    node->data.integer_literal.value = value;
    return node;
}

ASTNode* create_float_literal_node(double value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FLOAT_LITERAL;
    node->data.float_literal.value = value;
    return node;
}

ASTNode* create_string_literal_node(const char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING_LITERAL;
    node->data.string_literal.value = strdup(value);
    return node;
}

ASTNode* create_boolean_literal_node(bool value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BOOLEAN_LITERAL;
    node->data.boolean_literal.value = value;
    return node;
}

ASTNode* create_binary_expression_node(ASTNode* left, const char* operator, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_EXPRESSION;
    node->data.binary_expression.left = left;
    node->data.binary_expression.operator = strdup(operator);
    node->data.binary_expression.right = right;
    return node;
}

ASTNode* create_unary_expression_node(const char* operator, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_UNARY_EXPRESSION;
    node->data.unary_expression.operator = strdup(operator);
    node->data.unary_expression.right = right;
    return node;
}

ASTNode* create_expression_statement_node(ASTNode* expression) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_EXPRESSION_STATEMENT;
    node->data.expression_statement.expression = expression;
    return node;
}

ASTNode* create_block_statement_node(ASTNode** statements, size_t statement_count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK_STATEMENT;
    node->data.block_statement.statements = statements;
    node->data.block_statement.statement_count = statement_count;
    return node;
}

void free_ast_node(ASTNode* node) {
    if (!node) return;
    // 各ノード型ごとに必要なら再帰的に解放（ここでは簡易実装）
    switch (node->type) {
        case NODE_VARIABLE:
            free(node->data.variable.name);
            break;
        case NODE_FUNCTION:
            free(node->data.function.name);
            // parameters, body なども必要に応じて解放
            break;
        case NODE_LET_STATEMENT:
            free(node->data.let_statement.name);
            break;
        case NODE_FUNCTION_CALL:
            free(node->data.function_call.name);
            break;
        case NODE_ASSIGNMENT:
            free(node->data.assignment.name);
            break;
        case NODE_VARIABLE_REFERENCE:
            free(node->data.variable_reference.name);
            break;
        case NODE_STRING_LITERAL:
            free(node->data.string_literal.value);
            break;
        case NODE_BINARY_EXPRESSION:
            free(node->data.binary_expression.operator);
            break;
        case NODE_UNARY_EXPRESSION:
            free(node->data.unary_expression.operator);
            break;
        default:
            break;
    }
    free(node);
}

AST* create_ast(void) {
    AST* ast = (AST*)malloc(sizeof(AST));
    if (ast == NULL) {
        return NULL;
    }
    ast->functions = NULL;
    ast->function_count = 0;
    ast->type_definitions = NULL;
    ast->type_definition_count = 0;
    return ast;
}

void free_ast(AST* ast) {
    if (!ast) return;
    
    // Free functions
    for (size_t i = 0; i < ast->function_count; i++) {
        Function* func = ast->functions[i];
        free(func->name);
        for (size_t j = 0; j < func->parameter_count; j++) {
            free(func->parameters[j]->name);
            free(func->parameters[j]);
        }
        free(func->parameters);
        free_ast_node(func->body);
        free(func);
    }
    free(ast->functions);
    
    // Free type definitions
    for (size_t i = 0; i < ast->type_definition_count; i++) {
        TypeDefinition* type_def = ast->type_definitions[i];
        free(type_def->name);
        free_type(type_def->type);
        free(type_def);
    }
    free(ast->type_definitions);
    
    free(ast);
}

void ast_add_function(AST* ast, Function* function) {
    ast->functions = realloc(ast->functions, (ast->function_count + 1) * sizeof(Function*));
    ast->functions[ast->function_count++] = function;
}

void ast_add_type_definition(AST* ast, TypeDefinition* type_def) {
    ast->type_definitions = realloc(ast->type_definitions, (ast->type_definition_count + 1) * sizeof(TypeDefinition*));
    ast->type_definitions[ast->type_definition_count++] = type_def;
} 