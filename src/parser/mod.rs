use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::lexer::{Lexer, Token};
use crate::type_system::Type;

pub struct Parser<'a> {
    lexer: Lexer<'a>,
    current: Option<Token>,
    peek: Option<Token>,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: Lexer<'a>) -> Result<Self> {
        let mut parser = Self {
            lexer,
            current: None,
            peek: None,
        };
        parser.next()?;
        parser.next()?;
        Ok(parser)
    }

    fn next(&mut self) -> Result<()> {
        self.current = self.peek.take();
        self.peek = self.lexer.next().cloned();
        Ok(())
    }

    fn current(&self) -> Option<&Token> {
        self.current.as_ref()
    }

    fn peek(&self) -> Option<&Token> {
        self.peek.as_ref()
    }

    pub fn parse_program(&mut self) -> Result<Program> {
        let mut items = Vec::new();
        while self.current().is_some() {
            items.push(self.parse_item()?);
        }
        Ok(Program { items })
    }

    fn parse_item(&mut self) -> Result<Item> {
        match self.current() {
            Some(Token::Fn) => Ok(Item::Function(self.parse_function()?)),
            Some(Token::Identifier(_)) => {
                // 構造体、トレイト、モジュール、マクロの解析
                unimplemented!()
            }
            _ => Err(SlangError::Syntax("Expected item".to_string())),
        }
    }

    fn parse_function(&mut self) -> Result<Function> {
        self.next()?; // Skip 'fn'
        let name = match self.current() {
            Some(Token::Identifier(name)) => name.clone(),
            _ => return Err(SlangError::Syntax("Expected function name".to_string())),
        };
        self.next()?;

        let params = self.parse_parameters()?;
        let return_type = self.parse_return_type()?;
        let body = self.parse_block()?;
        let priority = self.parse_function_priority()?;

        Ok(Function {
            name,
            params,
            return_type,
            body,
            priority,
        })
    }

    fn parse_parameters(&mut self) -> Result<Vec<Parameter>> {
        let mut params = Vec::new();
        match self.current() {
            Some(Token::LeftParen) => {
                self.next()?;
                while let Some(token) = self.current() {
                    match token {
                        Token::RightParen => {
                            self.next()?;
                            break;
                        }
                        Token::Identifier(name) => {
                            let name = name.clone();
                            self.next()?;
                            match self.current() {
                                Some(Token::Colon) => {
                                    self.next()?;
                                    let type_ = self.parse_type()?;
                                    params.push(Parameter { name, type_ });
                                    match self.current() {
                                        Some(Token::Comma) => self.next()?,
                                        Some(Token::RightParen) => continue,
                                        _ => return Err(SlangError::Syntax("Expected ',' or ')'".to_string())),
                                    }
                                }
                                _ => return Err(SlangError::Syntax("Expected ':'".to_string())),
                            }
                        }
                        _ => return Err(SlangError::Syntax("Expected parameter name".to_string())),
                    }
                }
            }
            _ => return Err(SlangError::Syntax("Expected '('".to_string())),
        }
        Ok(params)
    }

    fn parse_return_type(&mut self) -> Result<Type> {
        match self.current() {
            Some(Token::Arrow) => {
                self.next()?;
                self.parse_type()
            }
            _ => Ok(Type::Void),
        }
    }

    fn parse_type(&mut self) -> Result<Type> {
        match self.current() {
            Some(Token::Identifier(name)) => {
                let type_ = match name.as_str() {
                    "int" => Type::Int,
                    "float" => Type::Float,
                    "bool" => Type::Bool,
                    "char" => Type::Char,
                    "string" => Type::String,
                    "void" => Type::Void,
                    _ => return Err(SlangError::Syntax(format!("Unknown type: {}", name))),
                };
                self.next()?;
                Ok(type_)
            }
            _ => Err(SlangError::Syntax("Expected type".to_string())),
        }
    }

    fn parse_block(&mut self) -> Result<Block> {
        let mut statements = Vec::new();
        match self.current() {
            Some(Token::LeftBrace) => {
                self.next()?;
                while let Some(token) = self.current() {
                    match token {
                        Token::RightBrace => {
                            self.next()?;
                            break;
                        }
                        _ => statements.push(self.parse_statement()?),
                    }
                }
            }
            _ => return Err(SlangError::Syntax("Expected '{'".to_string())),
        }
        Ok(Block { statements })
    }

    fn parse_statement(&mut self) -> Result<Statement> {
        match self.current() {
            Some(Token::Let) => Ok(Statement::Let(self.parse_let_statement()?)),
            Some(Token::If) => Ok(Statement::If(self.parse_if_statement()?)),
            Some(Token::Match) => Ok(Statement::Match(self.parse_match_statement()?)),
            Some(Token::While) => Ok(Statement::While(self.parse_while_statement()?)),
            Some(Token::For) => Ok(Statement::For(self.parse_for_statement()?)),
            Some(Token::Return) => Ok(Statement::Return(self.parse_return_statement()?)),
            Some(Token::VarTypePriority) | Some(Token::FunctionTypePriority) => {
                Ok(Statement::Priority(self.parse_priority_statement()?))
            }
            _ => Ok(Statement::Expression(self.parse_expression()?)),
        }
    }

    // 他のパースメソッドも同様に実装...
} 