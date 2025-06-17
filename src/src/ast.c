#include "../include/ast.h"
#include <stdlib.h>
#include <string.h>

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