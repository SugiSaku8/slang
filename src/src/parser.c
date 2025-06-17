#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/vector.h"
#include "../include/error.h"
#include <stdlib.h>
#include <string.h>

// 構文解析器の作成
Parser* parser_create(Lexer* lexer) {
    Parser* parser = malloc(sizeof(Parser));
    if (parser == NULL) return NULL;

    parser->lexer = lexer;
    parser->current = NULL;
    parser->previous = NULL;
    parser->errors = vector_create(sizeof(Error));
    
    if (parser->errors == NULL) {
        free(parser);
        return NULL;
    }

    return parser;
}

// 構文解析器の破棄
void parser_destroy(Parser* parser) {
    if (parser == NULL) return;
    
    // エラーメッセージを解放
    for (size_t i = 0; i < vector_size(parser->errors); i++) {
        Error* error = vector_get(parser->errors, i);
        free(error->message);
        free(error->file);
    }
    
    vector_destroy(parser->errors);
    free(parser);
}

// エラーの追加
static void parser_error(Parser* parser, ErrorType type, const char* message) {
    Error error;
    error.type = type;
    error.message = strdup(message);
    error.line = parser->current->line;
    error.column = parser->current->column;
    error.file = NULL; // TODO: ファイル名を設定
    
    vector_push(parser->errors, &error);
}

// 次のトークンを消費
static bool parser_advance(Parser* parser) {
    parser->previous = parser->current;
    parser->current = lexer_next_token(parser->lexer);
    return parser->current != NULL;
}

// 現在のトークンの種類を確認
static bool parser_check(Parser* parser, TokenType type) {
    if (parser->current == NULL) return false;
    return parser->current->type == type;
}

// 現在のトークンの種類を確認して消費
static bool parser_match(Parser* parser, TokenType type) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    return false;
}

// 現在のトークンの種類を確認して消費（エラー処理付き）
static bool parser_consume(Parser* parser, TokenType type, const char* message) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    
    parser_error(parser, ERROR_SYNTAX, message);
    return false;
}

// 式の解析
static SlangError parser_expression(Parser* parser, ASTNode** expr) {
    // TODO: 式の解析を実装
    return SLANG_SUCCESS;
}

// 文の解析
static SlangError parser_statement(Parser* parser, ASTNode** stmt) {
    if (parser_match(parser, TOKEN_LET)) {
        // 変数宣言
        // TODO: 変数宣言の解析を実装
    } else if (parser_match(parser, TOKEN_IF)) {
        // if文
        // TODO: if文の解析を実装
    } else if (parser_match(parser, TOKEN_WHILE)) {
        // while文
        // TODO: while文の解析を実装
    } else if (parser_match(parser, TOKEN_FOR)) {
        // for文
        // TODO: for文の解析を実装
    } else if (parser_match(parser, TOKEN_RETURN)) {
        // return文
        // TODO: return文の解析を実装
    } else if (parser_match(parser, TOKEN_BREAK)) {
        // break文
        // TODO: break文の解析を実装
    } else if (parser_match(parser, TOKEN_CONTINUE)) {
        // continue文
        // TODO: continue文の解析を実装
    } else {
        // 式文
        // TODO: 式文の解析を実装
    }
    
    return SLANG_SUCCESS;
}

// 関数宣言の解析
static SlangError parser_function(Parser* parser, ASTNode** func) {
    // TODO: 関数宣言の解析を実装
    return SLANG_SUCCESS;
}

// 構造体宣言の解析
static SlangError parser_struct(Parser* parser, ASTNode** struct_decl) {
    // TODO: 構造体宣言の解析を実装
    return SLANG_SUCCESS;
}

// 実装宣言の解析
static SlangError parser_impl(Parser* parser, ASTNode** impl_decl) {
    // TODO: 実装宣言の解析を実装
    return SLANG_SUCCESS;
}

// トレイト宣言の解析
static SlangError parser_trait(Parser* parser, ASTNode** trait_decl) {
    // TODO: トレイト宣言の解析を実装
    return SLANG_SUCCESS;
}

// 宣言の解析
static SlangError parser_declaration(Parser* parser, ASTNode** decl) {
    if (parser_match(parser, TOKEN_FN)) {
        return parser_function(parser, decl);
    } else if (parser_match(parser, TOKEN_STRUCT)) {
        return parser_struct(parser, decl);
    } else if (parser_match(parser, TOKEN_IMPL)) {
        return parser_impl(parser, decl);
    } else if (parser_match(parser, TOKEN_TRAIT)) {
        return parser_trait(parser, decl);
    } else {
        return parser_statement(parser, decl);
    }
}

// プログラムの解析
SlangError parser_parse(Parser* parser, ASTNode** ast) {
    // 最初のトークンを取得
    parser_advance(parser);
    
    // プログラムノードを作成
    *ast = ast_create_node(NODE_PROGRAM, 1, 1);
    if (*ast == NULL) {
        return SLANG_ERROR_INTERNAL;
    }
    
    // 宣言を解析
    Vector* declarations = vector_create(sizeof(ASTNode*));
    if (declarations == NULL) {
        ast_destroy_node(*ast);
        return SLANG_ERROR_INTERNAL;
    }
    
    while (!parser_check(parser, TOKEN_EOF)) {
        ASTNode* decl = NULL;
        SlangError error = parser_declaration(parser, &decl);
        
        if (error != SLANG_SUCCESS) {
            vector_destroy(declarations);
            ast_destroy_node(*ast);
            return error;
        }
        
        if (decl != NULL) {
            vector_push(declarations, &decl);
        }
    }
    
    (*ast)->as.program.declarations = declarations;
    return SLANG_SUCCESS;
}

// エラーの有無を確認
bool parser_had_error(Parser* parser) {
    return vector_size(parser->errors) > 0;
}

// エラーからの回復
void parser_synchronize(Parser* parser) {
    while (!parser_check(parser, TOKEN_EOF)) {
        if (parser->previous->type == TOKEN_SEMICOLON) return;
        
        switch (parser->current->type) {
            case TOKEN_FN:
            case TOKEN_STRUCT:
            case TOKEN_IMPL:
            case TOKEN_TRAIT:
            case TOKEN_LET:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_RETURN:
                return;
            default:
                break;
        }
        
        parser_advance(parser);
    }
}

Parser* create_parser(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (parser == NULL) {
        return NULL;
    }
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    return parser;
}

void free_parser(Parser* parser) {
    if (parser != NULL) {
        free(parser);
    }
}

ASTNode* parse_program(Parser* parser) {
    AST* ast = (AST*)malloc(sizeof(AST));
    if (ast == NULL) {
        return NULL;
    }
    ast->functions = NULL;
    ast->function_count = 0;
    ast->type_definitions = NULL;
    ast->type_definition_count = 0;

    while (!lexer_is_at_end(parser->lexer)) {
        Token token = lexer_peek_token(parser->lexer);
        
        if (token.type == TOKEN_FUNCTION) {
            Function function;
            SlangError* error = parser_parse_function(parser, &function);
            if (error != NULL) {
                free(ast);
                return NULL;
            }
            ast_add_function(ast, &function);
        } else if (token.type == TOKEN_TYPE) {
            TypeDefinition type_def;
            SlangError* error = parser_parse_type_definition(parser, &type_def);
            if (error != NULL) {
                free(ast);
                return NULL;
            }
            ast_add_type_definition(ast, &type_def);
        } else {
            free(ast);
            return NULL;
        }
    }
    return create_block_statement_node(NULL, 0);
}

SlangError* parser_parse_function(Parser* parser, Function* function) {
    Token token = lexer_next_token(parser->lexer);
    if (token.type != TOKEN_FUNCTION) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected 'fn' keyword");
    }

    char* name;
    SlangError* error = parser_parse_identifier(parser, &name);
    if (error != NULL) {
        return error;
    }
    function->name = name;

    error = parser_expect(parser, TOKEN_LEFT_PAREN);
    if (error != NULL) {
        return error;
    }

    // Parse parameters
    Vector* parameters = vector_new(sizeof(Parameter));
    token = lexer_peek_token(parser->lexer);
    
    if (token.type != TOKEN_RIGHT_PAREN) {
        while (1) {
            Parameter param;
            error = parser_parse_identifier(parser, &param.name);
            if (error != NULL) {
                vector_free(parameters);
                return error;
            }

            error = parser_expect(parser, TOKEN_COLON);
            if (error != NULL) {
                vector_free(parameters);
                return error;
            }

            error = parser_parse_type(parser, param.type_annotation);
            if (error != NULL) {
                vector_free(parameters);
                return error;
            }

            vector_push(parameters, &param);

            token = lexer_peek_token(parser->lexer);
            if (token.type == TOKEN_RIGHT_PAREN) {
                break;
            }

            error = parser_expect(parser, TOKEN_COMMA);
            if (error != NULL) {
                vector_free(parameters);
                return error;
            }
        }
    }

    error = parser_expect(parser, TOKEN_RIGHT_PAREN);
    if (error != NULL) {
        vector_free(parameters);
        return error;
    }

    error = parser_expect(parser, TOKEN_ARROW);
    if (error != NULL) {
        vector_free(parameters);
        return error;
    }

    error = parser_parse_type(parser, function->return_type);
    if (error != NULL) {
        vector_free(parameters);
        return error;
    }

    // Parse priority
    token = lexer_peek_token(parser->lexer);
    if (token.type == TOKEN_PRIORITY) {
        lexer_next_token(parser->lexer);
        error = parser_parse_integer(parser, &function->priority);
        if (error != NULL) {
            vector_free(parameters);
            return error;
        }
    } else {
        function->priority = 0;
    }

    // Parse body
    error = parser_parse_block(parser, &function->body);
    if (error != NULL) {
        vector_free(parameters);
        return error;
    }

    function->parameters = (Variable**)parameters;
    return NULL;
}

SlangError* parser_parse_type(Parser* parser, Type* type) {
    Token token = lexer_peek_token(parser->lexer);
    
    if (token.type == TOKEN_IDENTIFIER) {
        type->kind = TYPE_NAMED;
        type->data.named.name = strdup(token.value.string);
        lexer_next_token(parser->lexer);
        return NULL;
    }
    
    if (token.type == TOKEN_LEFT_BRACKET) {
        lexer_next_token(parser->lexer);
        type->kind = TYPE_ARRAY;
        type->data.array.element_type = (Type*)malloc(sizeof(Type));
        SlangError* error = parser_parse_type(parser, type->data.array.element_type);
        if (error != NULL) {
            return error;
        }
        return parser_expect(parser, TOKEN_RIGHT_BRACKET);
    }
    
    if (token.type == TOKEN_LEFT_PAREN) {
        lexer_next_token(parser->lexer);
        type->kind = TYPE_TUPLE;
        Vector* types = vector_new(sizeof(Type));
        
        token = lexer_peek_token(parser->lexer);
        if (token.type != TOKEN_RIGHT_PAREN) {
            while (1) {
                Type element_type;
                SlangError* error = parser_parse_type(parser, &element_type);
                if (error != NULL) {
                    vector_free(types);
                    return error;
                }
                vector_push(types, &element_type);
                
                token = lexer_peek_token(parser->lexer);
                if (token.type == TOKEN_RIGHT_PAREN) {
                    break;
                }
                
                error = parser_expect(parser, TOKEN_COMMA);
                if (error != NULL) {
                    vector_free(types);
                    return error;
                }
            }
        }
        
        type->data.tuple.types = (Type**)types;
        type->data.tuple.type_count = types->size;
        return parser_expect(parser, TOKEN_RIGHT_PAREN);
    }
    
    return slang_error_new(SLANG_ERROR_SYNTAX, "Expected type");
}

SlangError* parser_parse_type_definition(Parser* parser, TypeDefinition* type_def) {
    Token token = lexer_next_token(parser->lexer);
    if (token.type != TOKEN_TYPE) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected 'type' keyword");
    }

    SlangError* error = parser_parse_identifier(parser, &type_def->name);
    if (error != NULL) {
        return error;
    }

    error = parser_expect(parser, TOKEN_EQUAL);
    if (error != NULL) {
        return error;
    }

    error = parser_parse_type(parser, type_def->type);
    if (error != NULL) {
        return error;
    }

    return NULL;
}

SlangError* parser_parse_identifier(Parser* parser, char** name) {
    Token token = lexer_next_token(parser->lexer);
    if (token.type != TOKEN_IDENTIFIER) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected identifier");
    }
    *name = strdup(token.value.string);
    return NULL;
}

SlangError* parser_parse_integer(Parser* parser, int* value) {
    Token token = lexer_next_token(parser->lexer);
    if (token.type != TOKEN_NUMBER) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected number");
    }
    *value = (int)token.value.number;
    return NULL;
}

SlangError* parser_parse_block(Parser* parser, ASTNode** block) {
    Token token = lexer_next_token(parser->lexer);
    if (token.type != TOKEN_LEFT_BRACE) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected '{'");
    }

    Vector* statements = vector_new(sizeof(ASTNode*));
    while (1) {
        token = lexer_peek_token(parser->lexer);
        if (token.type == TOKEN_RIGHT_BRACE) {
            lexer_next_token(parser->lexer);
            break;
        }

        ASTNode* statement;
        SlangError* error = parser_parse_statement(parser, &statement);
        if (error != NULL) {
            vector_free(statements);
            return error;
        }

        vector_push(statements, &statement);
    }

    *block = create_block_statement_node((ASTNode**)statements->data, statements->size);
    vector_free(statements);
    return NULL;
}

SlangError* parser_expect(Parser* parser, TokenType expected) {
    Token token = lexer_next_token(parser->lexer);
    if (token.type != expected) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Unexpected token");
    }
    return NULL;
}

SlangError* parser_parse_statement(Parser* parser, ASTNode** statement) {
    SlangError* error = NULL;
    Token token = lexer_peek_token(parser->lexer);
    
    if (token.type == TOKEN_VAR) {
        lexer_next_token(parser->lexer);
        
        char* name;
        error = parser_parse_identifier(parser, &name);
        if (error != NULL) {
            return error;
        }
        
        Type* type = NULL;
        token = lexer_peek_token(parser->lexer);
        if (token.type == TOKEN_COLON) {
            lexer_next_token(parser->lexer);
            type = (Type*)malloc(sizeof(Type));
            error = parser_parse_type(parser, type);
            if (error != NULL) {
                free(name);
                return error;
            }
        }
        
        error = parser_expect(parser, TOKEN_EQUAL);
        if (error != NULL) {
            free(name);
            if (type) free_type(type);
            return error;
        }
        
        ASTNode* initializer;
        error = parser_parse_expression(parser, &initializer);
        if (error != NULL) {
            free(name);
            if (type) free_type(type);
            return error;
        }
        
        *statement = create_let_statement_node(name, type, initializer);
        return NULL;
    }
    
    if (token.type == TOKEN_RETURN) {
        lexer_next_token(parser->lexer);
        
        ASTNode* value = NULL;
        token = lexer_peek_token(parser->lexer);
        if (token.type != TOKEN_SEMICOLON) {
            error = parser_parse_expression(parser, &value);
            if (error != NULL) {
                return error;
            }
        }
        
        *statement = create_expression_statement_node(value);
        return NULL;
    }
    
    // Expression statement
    error = parser_parse_expression(parser, statement);
    if (error != NULL) {
        return error;
    }
    
    return parser_expect(parser, TOKEN_SEMICOLON);
}

SlangError* parser_parse_expression(Parser* parser, ASTNode** expression) {
    Token token = lexer_peek_token(parser->lexer);
    
    if (token.type == TOKEN_IDENTIFIER) {
        char* name = strdup(token.value.string);
        lexer_next_token(parser->lexer);
        
        token = lexer_peek_token(parser->lexer);
        if (token.type == TOKEN_LEFT_PAREN) {
            lexer_next_token(parser->lexer);
            
            Vector* arguments = vector_new(sizeof(ASTNode*));
            token = lexer_peek_token(parser->lexer);
            
            if (token.type != TOKEN_RIGHT_PAREN) {
                while (1) {
                    ASTNode* argument;
                    SlangError* error = parser_parse_expression(parser, &argument);
                    if (error != NULL) {
                        vector_free(arguments);
                        free(name);
                        return error;
                    }
                    
                    vector_push(arguments, &argument);
                    
                    token = lexer_peek_token(parser->lexer);
                    if (token.type == TOKEN_RIGHT_PAREN) {
                        break;
                    }
                    
                    error = parser_expect(parser, TOKEN_COMMA);
                    if (error != NULL) {
                        vector_free(arguments);
                        free(name);
                        return error;
                    }
                }
            }
            
            lexer_next_token(parser->lexer);
            *expression = create_function_call_node(name, (ASTNode**)arguments->data, arguments->size);
            vector_free(arguments);
            return NULL;
        }
        
        *expression = create_variable_reference_node(name);
        return NULL;
    }
    
    if (token.type == TOKEN_NUMBER) {
        int value = (int)token.value.number;
        lexer_next_token(parser->lexer);
        *expression = create_integer_literal_node(value);
        return NULL;
    }
    
    if (token.type == TOKEN_STRING) {
        char* value = strdup(token.value.string);
        lexer_next_token(parser->lexer);
        *expression = create_string_literal_node(value);
        return NULL;
    }
    
    if (token.type == TOKEN_TRUE || token.type == TOKEN_FALSE) {
        bool value = token.type == TOKEN_TRUE;
        lexer_next_token(parser->lexer);
        *expression = create_boolean_literal_node(value);
        return NULL;
    }
    
    return slang_error_new(SLANG_ERROR_SYNTAX, "Expected expression");
} 