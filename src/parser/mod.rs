use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::lexer::{Lexer, Token};
use crate::type_system::Type;
use std::ops::Range;

pub struct Parser<'a> {
    lexer: Lexer<'a>,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: Lexer<'a>) -> Self {
        Self { lexer }
    }

    pub fn parse(&mut self) -> Result<AST> {
        let mut ast = AST::new();
        while let Some(token) = self.lexer.peek() {
            match token {
                Token::Function => {
                    let function = self.parse_function()?;
                    ast.add_function(function);
                }
                Token::Type => {
                    let type_def = self.parse_type_definition()?;
                    ast.add_type_definition(type_def);
                }
                _ => return Err(SlangError::Syntax(format!("Unexpected token: {:?}", token))),
            }
        }
        Ok(ast)
    }

    fn parse_function(&mut self) -> Result<Function> {
        self.expect(Token::Function)?;
        let name = self.parse_identifier()?;
        self.expect(Token::LParen)?;
        let mut parameters = Vec::new();
        if let Some(token) = self.lexer.peek() {
            if token != &Token::RParen {
                loop {
                    let param_name = self.parse_identifier()?;
                    self.expect(Token::Colon)?;
                    let param_type = self.parse_type()?;
                    parameters.push(Parameter {
                        name: param_name,
                        type_annotation: param_type,
                    });
                    if let Some(token) = self.lexer.peek() {
                        if token == &Token::RParen {
                            break;
                        }
                        self.expect(Token::Comma)?;
                    } else {
                        return Err(SlangError::Syntax("Expected ')' or ','".to_string()));
                    }
                }
            }
        }
        self.expect(Token::RParen)?;
        self.expect(Token::Arrow)?;
        let return_type = self.parse_type()?;
        let priority = if let Some(Token::Priority) = self.lexer.peek() {
            self.lexer.next();
            self.parse_integer()?
        } else {
            0
        };
        let body = self.parse_block()?;
        Ok(Function {
            name,
            parameters,
            return_type,
            priority,
            body,
        })
    }

    fn parse_type(&mut self) -> Result<Type> {
        match self.lexer.peek() {
            Some(Token::Identifier(name)) => {
                let name = name.clone();
                self.lexer.next();
                Ok(Type::Named(name))
            }
            Some(Token::LBracket) => {
                self.lexer.next();
                let element_type = Box::new(self.parse_type()?);
                self.expect(Token::RBracket)?;
                Ok(Type::Array(element_type))
            }
            Some(Token::LParen) => {
                self.lexer.next();
                let mut types = Vec::new();
                if let Some(token) = self.lexer.peek() {
                    if token != &Token::RParen {
                        loop {
                            types.push(self.parse_type()?);
                            if let Some(token) = self.lexer.peek() {
                                if token == &Token::RParen {
                                    break;
                                }
                                self.expect(Token::Comma)?;
                            } else {
                                return Err(SlangError::Syntax("Expected ')' or ','".to_string()));
                            }
                        }
                    }
                }
                self.expect(Token::RParen)?;
                Ok(Type::Tuple(types))
            }
            _ => Err(SlangError::Syntax(format!("Unexpected token in type: {:?}", self.lexer.peek()))),
        }
    }

    fn parse_type_definition(&mut self) -> Result<TypeDefinition> {
        self.expect(Token::Type)?;
        let name = self.parse_identifier()?;
        self.expect(Token::Equals)?;
        let fields = vec![]; // TODO: parse fields properly
        self.expect(Token::Semicolon)?;
        Ok(TypeDefinition { name, fields })
    }

    fn parse_pattern(&mut self) -> Result<Pattern> {
        match self.lexer.peek() {
            Some(Token::Identifier(name)) => {
                let name = name.clone();
                self.lexer.next();
                Ok(Pattern::Identifier(name))
            }
            Some(Token::Underscore) => {
                self.lexer.next();
                Ok(Pattern::Wildcard)
            }
            Some(Token::LParen) => {
                self.lexer.next();
                let mut patterns = Vec::new();
                if let Some(token) = self.lexer.peek() {
                    if token != &Token::RParen {
                        loop {
                            patterns.push(self.parse_pattern()?);
                            if let Some(token) = self.lexer.peek() {
                                if token == &Token::RParen {
                                    break;
                                }
                                self.expect(Token::Comma)?;
                            } else {
                                return Err(SlangError::Syntax("Expected ')' or ','".to_string()));
                            }
                        }
                    }
                }
                self.expect(Token::RParen)?;
                Ok(Pattern::Tuple(patterns))
            }
            _ => Err(SlangError::Syntax(format!("Unexpected token in pattern: {:?}", self.lexer.peek()))),
        }
    }

    fn parse_identifier(&mut self) -> Result<String> {
        match self.lexer.peek() {
            Some(Token::Identifier(name)) => {
                let name = name.clone();
                self.lexer.next();
                Ok(name)
            }
            _ => Err(SlangError::Syntax(format!("Expected identifier, got {:?}", self.lexer.peek()))),
        }
    }

    fn parse_string(&mut self) -> Result<String> {
        match self.lexer.peek() {
            Some(Token::StringLiteral(value)) => {
                let value = value.clone();
                self.lexer.next();
                Ok(value)
            }
            _ => Err(SlangError::Syntax(format!("Expected string literal, got {:?}", self.lexer.peek()))),
        }
    }

    fn parse_integer(&mut self) -> Result<i32> {
        match self.lexer.peek() {
            Some(Token::IntegerLiteral(value)) => {
                let value = *value;
                self.lexer.next();
                Ok(value as i32)
            }
            _ => Err(SlangError::Syntax(format!("Expected integer literal, got {:?}", self.lexer.peek()))),
        }
    }

    fn parse_float(&mut self) -> Result<f64> {
        match self.lexer.peek() {
            Some(Token::FloatLiteral(value)) => {
                let value = *value;
                self.lexer.next();
                Ok(value)
            }
            _ => Err(SlangError::Syntax(format!("Expected float literal, got {:?}", self.lexer.peek()))),
        }
    }

    fn parse_block(&mut self) -> Result<Block> {
        self.expect(Token::LBrace)?;
        let mut statements = Vec::new();
        while let Some(token) = self.lexer.peek() {
            if token == &Token::RBrace {
                break;
            }
            statements.push(self.parse_statement()?);
        }
        self.expect(Token::RBrace)?;
        Ok(Block { statements })
    }

    fn parse_statement(&mut self) -> Result<Statement> {
        match self.lexer.peek() {
            Some(Token::Let) => {
                self.lexer.next();
                let name = self.parse_identifier()?;
                let type_annotation = if let Some(Token::Colon) = self.lexer.peek() {
                    self.lexer.next();
                    Some(self.parse_type()?)
                } else {
                    None
                };
                self.expect(Token::Equals)?;
                let value = Box::new(self.parse_expression()?);
                self.expect(Token::Semicolon)?;
                Ok(Statement::Let(LetStatement {
                    name,
                    type_annotation,
                    value,
                }))
            }
            Some(Token::Return) => {
                self.lexer.next();
                let value = if let Some(token) = self.lexer.peek() {
                    if token != &Token::Semicolon {
                        Some(Box::new(self.parse_expression()?))
                    } else {
                        None
                    }
                } else {
                    None
                };
                self.expect(Token::Semicolon)?;
                Ok(Statement::Return(ReturnStatement { value }))
            }
            _ => {
                let expr = Box::new(self.parse_expression()?);
                self.expect(Token::Semicolon)?;
                Ok(Statement::Expression(expr))
            }
        }
    }

    fn parse_expression(&mut self) -> Result<Expression> {
        match self.lexer.peek() {
            Some(Token::Identifier(name)) => {
                let name = name.clone();
                self.lexer.next();
                if let Some(Token::LParen) = self.lexer.peek() {
                    self.lexer.next();
                    let mut arguments = Vec::new();
                    if let Some(token) = self.lexer.peek() {
                        if token != &Token::RParen {
                            loop {
                                arguments.push(Box::new(self.parse_expression()?));
                                if let Some(token) = self.lexer.peek() {
                                    if token == &Token::RParen {
                                        break;
                                    }
                                    self.expect(Token::Comma)?;
                                } else {
                                    return Err(SlangError::Syntax("Expected ')' or ','".to_string()));
                                }
                            }
                        }
                    }
                    self.expect(Token::RParen)?;
                    Ok(Expression::Call(Box::new(CallExpression {
                        function: name,
                        arguments,
                    })))
                } else {
                    Ok(Expression::Identifier(name))
                }
            }
            Some(Token::StringLiteral(value)) => {
                let value = value.clone();
                self.lexer.next();
                Ok(Expression::Literal(Literal::String(value)))
            }
            Some(Token::IntegerLiteral(value)) => {
                let value = *value;
                self.lexer.next();
                Ok(Expression::Literal(Literal::Int(value)))
            }
            Some(Token::FloatLiteral(value)) => {
                let value = *value;
                self.lexer.next();
                Ok(Expression::Literal(Literal::Float(value)))
            }
            Some(Token::True) => {
                self.lexer.next();
                Ok(Expression::Literal(Literal::Bool(true)))
            }
            Some(Token::False) => {
                self.lexer.next();
                Ok(Expression::Literal(Literal::Bool(false)))
            }
            Some(Token::Null) => {
                self.lexer.next();
                Ok(Expression::Literal(Literal::Null))
            }
            _ => Err(SlangError::Syntax(format!("Unexpected token in expression: {:?}", self.lexer.peek()))),
        }
    }

    fn expect(&mut self, expected: Token) -> Result<()> {
        match self.lexer.peek() {
            Some(token) if token == &expected => {
                self.lexer.next();
                Ok(())
            }
            Some(token) => Err(SlangError::Syntax(format!(
                "Expected {:?}, got {:?}",
                expected, token
            ))),
            None => Err(SlangError::Syntax(format!("Expected {:?}, got EOF", expected))),
        }
    }
} 