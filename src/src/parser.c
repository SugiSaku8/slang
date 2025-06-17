#include "parser.h"
#include "../include/ast.h"
#include "../include/vector.h"
#include "../include/error.h"
#include <stdlib.h>
#include <string.h>

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