#include "parser.h"
#include <stdlib.h>
#include <string.h>

Parser* parser_new(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (parser == NULL) {
        return NULL;
    }
    parser->lexer = lexer;
    return parser;
}

void parser_free(Parser* parser) {
    if (parser != NULL) {
        free(parser);
    }
}

SlangError* parser_parse(Parser* parser, AST* ast) {
    while (!lexer_is_at_end(parser->lexer)) {
        Token token = lexer_peek_token(parser->lexer);
        
        if (token.type == TOKEN_FUNCTION) {
            Function function;
            SlangError* error = parser_parse_function(parser, &function);
            if (error != NULL) {
                return error;
            }
            ast_add_function(ast, &function);
        } else if (token.type == TOKEN_TYPE) {
            TypeDefinition type_def;
            SlangError* error = parser_parse_type_definition(parser, &type_def);
            if (error != NULL) {
                return error;
            }
            ast_add_type_definition(ast, &type_def);
        } else {
            return slang_error_new(SLANG_ERROR_SYNTAX, "Unexpected token");
        }
    }
    return NULL;
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

    error = parser_expect(parser, TOKEN_LPAREN);
    if (error != NULL) {
        return error;
    }

    // Parse parameters
    Vector* parameters = vector_new(sizeof(Parameter));
    token = lexer_peek_token(parser->lexer);
    
    if (token.type != TOKEN_RPAREN) {
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

            error = parser_parse_type(parser, &param.type_annotation);
            if (error != NULL) {
                vector_free(parameters);
                return error;
            }

            vector_push(parameters, &param);

            token = lexer_peek_token(parser->lexer);
            if (token.type == TOKEN_RPAREN) {
                break;
            }

            error = parser_expect(parser, TOKEN_COMMA);
            if (error != NULL) {
                vector_free(parameters);
                return error;
            }
        }
    }

    error = parser_expect(parser, TOKEN_RPAREN);
    if (error != NULL) {
        vector_free(parameters);
        return error;
    }

    error = parser_expect(parser, TOKEN_ARROW);
    if (error != NULL) {
        vector_free(parameters);
        return error;
    }

    error = parser_parse_type(parser, &function->return_type);
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

    function->parameters = parameters;
    return NULL;
}

SlangError* parser_parse_type(Parser* parser, Type* type) {
    Token token = lexer_peek_token(parser->lexer);
    
    if (token.type == TOKEN_IDENTIFIER) {
        type->kind = TYPE_NAMED;
        type->named.name = strdup(token.value.string);
        lexer_next_token(parser->lexer);
        return NULL;
    }
    
    if (token.type == TOKEN_LBRACKET) {
        lexer_next_token(parser->lexer);
        type->kind = TYPE_ARRAY;
        type->array.element_type = (Type*)malloc(sizeof(Type));
        SlangError* error = parser_parse_type(parser, type->array.element_type);
        if (error != NULL) {
            return error;
        }
        return parser_expect(parser, TOKEN_RBRACKET);
    }
    
    if (token.type == TOKEN_LPAREN) {
        lexer_next_token(parser->lexer);
        type->kind = TYPE_TUPLE;
        type->tuple.types = vector_new(sizeof(Type));
        
        token = lexer_peek_token(parser->lexer);
        if (token.type != TOKEN_RPAREN) {
            while (1) {
                Type element_type;
                SlangError* error = parser_parse_type(parser, &element_type);
                if (error != NULL) {
                    vector_free(type->tuple.types);
                    return error;
                }
                vector_push(type->tuple.types, &element_type);
                
                token = lexer_peek_token(parser->lexer);
                if (token.type == TOKEN_RPAREN) {
                    break;
                }
                
                error = parser_expect(parser, TOKEN_COMMA);
                if (error != NULL) {
                    vector_free(type->tuple.types);
                    return error;
                }
            }
        }
        
        return parser_expect(parser, TOKEN_RPAREN);
    }
    
    return slang_error_new(SLANG_ERROR_SYNTAX, "Unexpected token in type");
}

SlangError* parser_parse_type_definition(Parser* parser, TypeDefinition* type_def) {
    SlangError* error = parser_expect(parser, TOKEN_TYPE);
    if (error != NULL) {
        return error;
    }

    error = parser_parse_identifier(parser, &type_def->name);
    if (error != NULL) {
        return error;
    }

    error = parser_expect(parser, TOKEN_EQUALS);
    if (error != NULL) {
        return error;
    }

    // TODO: Parse fields properly
    type_def->fields = vector_new(sizeof(Field));

    error = parser_expect(parser, TOKEN_SEMICOLON);
    if (error != NULL) {
        vector_free(type_def->fields);
        return error;
    }

    return NULL;
}

SlangError* parser_parse_pattern(Parser* parser, Pattern* pattern) {
    Token token = lexer_peek_token(parser->lexer);
    
    if (token.type == TOKEN_IDENTIFIER) {
        pattern->kind = PATTERN_IDENTIFIER;
        pattern->identifier = strdup(token.value.string);
        lexer_next_token(parser->lexer);
        return NULL;
    }
    
    if (token.type == TOKEN_UNDERSCORE) {
        pattern->kind = PATTERN_WILDCARD;
        lexer_next_token(parser->lexer);
        return NULL;
    }
    
    if (token.type == TOKEN_LPAREN) {
        lexer_next_token(parser->lexer);
        pattern->kind = PATTERN_TUPLE;
        pattern->tuple_patterns = vector_new(sizeof(Pattern));
        
        token = lexer_peek_token(parser->lexer);
        if (token.type != TOKEN_RPAREN) {
            while (1) {
                Pattern element_pattern;
                SlangError* error = parser_parse_pattern(parser, &element_pattern);
                if (error != NULL) {
                    vector_free(pattern->tuple_patterns);
                    return error;
                }
                vector_push(pattern->tuple_patterns, &element_pattern);
                
                token = lexer_peek_token(parser->lexer);
                if (token.type == TOKEN_RPAREN) {
                    break;
                }
                
                error = parser_expect(parser, TOKEN_COMMA);
                if (error != NULL) {
                    vector_free(pattern->tuple_patterns);
                    return error;
                }
            }
        }
        
        return parser_expect(parser, TOKEN_RPAREN);
    }
    
    return slang_error_new(SLANG_ERROR_SYNTAX, "Unexpected token in pattern");
}

SlangError* parser_parse_identifier(Parser* parser, char** identifier) {
    Token token = lexer_peek_token(parser->lexer);
    if (token.type != TOKEN_IDENTIFIER) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected identifier");
    }
    *identifier = strdup(token.value.string);
    lexer_next_token(parser->lexer);
    return NULL;
}

SlangError* parser_parse_string(Parser* parser, char** string) {
    Token token = lexer_peek_token(parser->lexer);
    if (token.type != TOKEN_STRING_LITERAL) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected string literal");
    }
    *string = strdup(token.value.string);
    lexer_next_token(parser->lexer);
    return NULL;
}

SlangError* parser_parse_integer(Parser* parser, int32_t* integer) {
    Token token = lexer_peek_token(parser->lexer);
    if (token.type != TOKEN_INTEGER_LITERAL) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected integer literal");
    }
    *integer = (int32_t)token.value.integer;
    lexer_next_token(parser->lexer);
    return NULL;
}

SlangError* parser_parse_float(Parser* parser, double* floating) {
    Token token = lexer_peek_token(parser->lexer);
    if (token.type != TOKEN_FLOAT_LITERAL) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Expected float literal");
    }
    *floating = token.value.floating;
    lexer_next_token(parser->lexer);
    return NULL;
}

SlangError* parser_parse_block(Parser* parser, Block* block) {
    SlangError* error = parser_expect(parser, TOKEN_LBRACE);
    if (error != NULL) {
        return error;
    }

    block->statements = vector_new(sizeof(Statement));
    
    while (1) {
        Token token = lexer_peek_token(parser->lexer);
        if (token.type == TOKEN_RBRACE) {
            break;
        }
        
        Statement statement;
        error = parser_parse_statement(parser, &statement);
        if (error != NULL) {
            vector_free(block->statements);
            return error;
        }
        
        vector_push(block->statements, &statement);
    }
    
    return parser_expect(parser, TOKEN_RBRACE);
}

SlangError* parser_parse_statement(Parser* parser, Statement* statement) {
    Token token = lexer_peek_token(parser->lexer);
    
    if (token.type == TOKEN_LET) {
        lexer_next_token(parser->lexer);
        statement->kind = STATEMENT_LET;
        
        LetStatement* let_stmt = &statement->let_statement;
        SlangError* error = parser_parse_identifier(parser, &let_stmt->name);
        if (error != NULL) {
            return error;
        }
        
        token = lexer_peek_token(parser->lexer);
        if (token.type == TOKEN_COLON) {
            lexer_next_token(parser->lexer);
            let_stmt->type_annotation = (Type*)malloc(sizeof(Type));
            error = parser_parse_type(parser, let_stmt->type_annotation);
            if (error != NULL) {
                return error;
            }
        } else {
            let_stmt->type_annotation = NULL;
        }
        
        error = parser_expect(parser, TOKEN_EQUALS);
        if (error != NULL) {
            return error;
        }
        
        let_stmt->value = (Expression*)malloc(sizeof(Expression));
        error = parser_parse_expression(parser, let_stmt->value);
        if (error != NULL) {
            return error;
        }
        
        return parser_expect(parser, TOKEN_SEMICOLON);
    }
    
    if (token.type == TOKEN_RETURN) {
        lexer_next_token(parser->lexer);
        statement->kind = STATEMENT_RETURN;
        
        token = lexer_peek_token(parser->lexer);
        if (token.type != TOKEN_SEMICOLON) {
            statement->return_statement.value = (Expression*)malloc(sizeof(Expression));
            SlangError* error = parser_parse_expression(parser, statement->return_statement.value);
            if (error != NULL) {
                return error;
            }
        } else {
            statement->return_statement.value = NULL;
        }
        
        return parser_expect(parser, TOKEN_SEMICOLON);
    }
    
    // Expression statement
    statement->kind = STATEMENT_EXPRESSION;
    statement->expression_statement.expression = (Expression*)malloc(sizeof(Expression));
    SlangError* error = parser_parse_expression(parser, statement->expression_statement.expression);
    if (error != NULL) {
        return error;
    }
    
    return parser_expect(parser, TOKEN_SEMICOLON);
}

SlangError* parser_parse_expression(Parser* parser, Expression* expression) {
    // TODO: Implement expression parsing
    return slang_error_new(SLANG_ERROR_SYNTAX, "Expression parsing not implemented");
}

SlangError* parser_expect(Parser* parser, TokenType expected) {
    Token token = lexer_next_token(parser->lexer);
    if (token.type != expected) {
        return slang_error_new(SLANG_ERROR_SYNTAX, "Unexpected token");
    }
    return NULL;
} 