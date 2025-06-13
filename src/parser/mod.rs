use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::lexer::{Lexer, Token};
use crate::type_system::Type;
use std::ops::Range;

pub struct Parser<'a> {
    lexer: &'a Lexer<'a>,
    current: usize,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: &'a Lexer<'a>) -> Self {
        Parser {
            lexer,
            current: 0,
        }
    }

    pub fn parse(&mut self) -> Result<AST, String> {
        let mut ast = AST::new();

        while let Some(token) = self.peek() {
            match token {
                Token::Function => {
                    let function = self.parse_function()?;
                    ast.add_function(function);
                }
                Token::Type => {
                    let type_def = self.parse_type_definition()?;
                    ast.add_type_definition(type_def);
                }
                _ => return Err(format!("Unexpected token: {:?}", token)),
            }
        }

        Ok(ast)
    }

    fn parse_function(&mut self) -> Result<Function, String> {
        self.expect(Token::Function)?;
        let name = self.parse_identifier()?;
        let parameters = self.parse_parameters()?;
        let return_type = self.parse_return_type()?;
        let priority = self.parse_priority()?;
        let body = self.parse_block()?;

        Ok(Function {
            name,
            parameters,
            return_type,
            priority,
            body,
        })
    }

    fn parse_parameters(&mut self) -> Result<Vec<Parameter>, String> {
        self.expect(Token::LeftParen)?;
        let mut parameters = Vec::new();

        if self.peek() != Some(&Token::RightParen) {
            loop {
                let name = self.parse_identifier()?;
                self.expect(Token::Colon)?;
                let type_annotation = self.parse_type()?;
                parameters.push(Parameter {
                    name,
                    type_annotation,
                });

                if self.peek() == Some(&Token::RightParen) {
                    break;
                }
                self.expect(Token::Comma)?;
            }
        }

        self.expect(Token::RightParen)?;
        Ok(parameters)
    }

    fn parse_return_type(&mut self) -> Result<Type, String> {
        if self.peek() == Some(&Token::Arrow) {
            self.next();
            self.parse_type()
        } else {
            Ok(Type::Unit)
        }
    }

    fn parse_priority(&mut self) -> Result<Vec<i32>, String> {
        if self.peek() == Some(&Token::Priority) {
            self.next();
            self.expect(Token::Colon)?;
            let mut priorities = Vec::new();

            if self.peek() == Some(&Token::LeftBracket) {
                self.next();
                loop {
                    if self.peek() == Some(&Token::MostHigh) {
                        self.next();
                        priorities.push(i32::MAX);
                    } else {
                        let priority = self.parse_integer()?;
                        priorities.push(priority);
                    }

                    if self.peek() == Some(&Token::RightBracket) {
                        break;
                    }
                    self.expect(Token::Comma)?;
                }
                self.expect(Token::RightBracket)?;
            } else {
                let priority = self.parse_integer()?;
                priorities.push(priority);
            }

            Ok(priorities)
        } else {
            Ok(vec![0])
        }
    }

    fn parse_block(&mut self) -> Result<Vec<Statement>, String> {
        self.expect(Token::LeftBrace)?;
        let mut statements = Vec::new();

        while self.peek() != Some(&Token::RightBrace) {
            let statement = self.parse_statement()?;
            statements.push(statement);
        }

        self.expect(Token::RightBrace)?;
        Ok(statements)
    }

    fn parse_statement(&mut self) -> Result<Statement, String> {
        match self.peek() {
            Some(Token::Let) => {
                self.next();
                let name = self.parse_identifier()?;
                let type_annotation = if self.peek() == Some(&Token::Colon) {
                    self.next();
                    Some(self.parse_type()?)
                } else {
                    None
                };
                self.expect(Token::Equals)?;
                let value = self.parse_expression()?;
                self.expect(Token::Semicolon)?;
                Ok(Statement::Let(LetStatement {
                    name,
                    type_annotation,
                    value,
                }))
            }
            Some(Token::Return) => {
                self.next();
                let value = self.parse_expression()?;
                self.expect(Token::Semicolon)?;
                Ok(Statement::Return(value))
            }
            Some(Token::If) => {
                self.next();
                self.expect(Token::LeftParen)?;
                let condition = self.parse_expression()?;
                self.expect(Token::RightParen)?;
                let then_branch = self.parse_block()?;
                let else_branch = if self.peek() == Some(&Token::Else) {
                    self.next();
                    Some(self.parse_block()?)
                } else {
                    None
                };
                Ok(Statement::If(IfStatement {
                    condition,
                    then_branch,
                    else_branch,
                }))
            }
            Some(Token::While) => {
                self.next();
                self.expect(Token::LeftParen)?;
                let condition = self.parse_expression()?;
                self.expect(Token::RightParen)?;
                let body = self.parse_block()?;
                Ok(Statement::While(WhileStatement {
                    condition,
                    body,
                }))
            }
            Some(Token::For) => {
                self.next();
                self.expect(Token::LeftParen)?;
                let variable = self.parse_identifier()?;
                self.expect(Token::In)?;
                let iterator = self.parse_expression()?;
                self.expect(Token::RightParen)?;
                let body = self.parse_block()?;
                Ok(Statement::For(ForStatement {
                    variable,
                    iterator,
                    body,
                }))
            }
            Some(Token::Match) => {
                self.next();
                self.expect(Token::LeftParen)?;
                let value = self.parse_expression()?;
                self.expect(Token::RightParen)?;
                self.expect(Token::LeftBrace)?;
                let mut arms = Vec::new();
                while self.peek() != Some(&Token::RightBrace) {
                    let pattern = self.parse_pattern()?;
                    self.expect(Token::Arrow)?;
                    let body = self.parse_block()?;
                    arms.push(MatchArm { pattern, body });
                }
                self.expect(Token::RightBrace)?;
                Ok(Statement::Match(MatchStatement { value, arms }))
            }
            _ => {
                let expr = self.parse_expression()?;
                self.expect(Token::Semicolon)?;
                Ok(Statement::Expression(expr))
            }
        }
    }

    fn parse_expression(&mut self) -> Result<Expression, String> {
        self.parse_assignment()
    }

    fn parse_assignment(&mut self) -> Result<Expression, String> {
        let mut expr = self.parse_equality()?;

        while let Some(token) = self.peek() {
            match token {
                Token::Equals => {
                    self.next();
                    let value = self.parse_assignment()?;
                    expr = Expression::Assignment(Box::new(AssignmentExpression {
                        target: expr,
                        value,
                    }));
                }
                _ => break,
            }
        }

        Ok(expr)
    }

    fn parse_equality(&mut self) -> Result<Expression, String> {
        let mut expr = self.parse_comparison()?;

        while let Some(token) = self.peek() {
            match token {
                Token::EqualsEquals => {
                    self.next();
                    let right = self.parse_comparison()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::Equals,
                        right: Box::new(right),
                    });
                }
                Token::NotEquals => {
                    self.next();
                    let right = self.parse_comparison()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::NotEquals,
                        right: Box::new(right),
                    });
                }
                _ => break,
            }
        }

        Ok(expr)
    }

    fn parse_comparison(&mut self) -> Result<Expression, String> {
        let mut expr = self.parse_term()?;

        while let Some(token) = self.peek() {
            match token {
                Token::LessThan => {
                    self.next();
                    let right = self.parse_term()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::LessThan,
                        right: Box::new(right),
                    });
                }
                Token::GreaterThan => {
                    self.next();
                    let right = self.parse_term()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::GreaterThan,
                        right: Box::new(right),
                    });
                }
                Token::LessThanEquals => {
                    self.next();
                    let right = self.parse_term()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::LessThanEquals,
                        right: Box::new(right),
                    });
                }
                Token::GreaterThanEquals => {
                    self.next();
                    let right = self.parse_term()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::GreaterThanEquals,
                        right: Box::new(right),
                    });
                }
                _ => break,
            }
        }

        Ok(expr)
    }

    fn parse_term(&mut self) -> Result<Expression, String> {
        let mut expr = self.parse_factor()?;

        while let Some(token) = self.peek() {
            match token {
                Token::Plus => {
                    self.next();
                    let right = self.parse_factor()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::Add,
                        right: Box::new(right),
                    });
                }
                Token::Minus => {
                    self.next();
                    let right = self.parse_factor()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::Subtract,
                        right: Box::new(right),
                    });
                }
                _ => break,
            }
        }

        Ok(expr)
    }

    fn parse_factor(&mut self) -> Result<Expression, String> {
        let mut expr = self.parse_unary()?;

        while let Some(token) = self.peek() {
            match token {
                Token::Star => {
                    self.next();
                    let right = self.parse_unary()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::Multiply,
                        right: Box::new(right),
                    });
                }
                Token::Slash => {
                    self.next();
                    let right = self.parse_unary()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::Divide,
                        right: Box::new(right),
                    });
                }
                Token::Percent => {
                    self.next();
                    let right = self.parse_unary()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::Modulo,
                        right: Box::new(right),
                    });
                }
                _ => break,
            }
        }

        Ok(expr)
    }

    fn parse_unary(&mut self) -> Result<Expression, String> {
        match self.peek() {
            Some(Token::Minus) => {
                self.next();
                let right = self.parse_unary()?;
                Ok(Expression::UnaryOp(UnaryOpExpression {
                    op: UnaryOperator::Negate,
                    right: Box::new(right),
                }))
            }
            Some(Token::Not) => {
                self.next();
                let right = self.parse_unary()?;
                Ok(Expression::UnaryOp(UnaryOpExpression {
                    op: UnaryOperator::Not,
                    right: Box::new(right),
                }))
            }
            _ => self.parse_primary(),
        }
    }

    fn parse_primary(&mut self) -> Result<Expression, String> {
        match self.peek() {
            Some(Token::Identifier(_)) => {
                let name = self.parse_identifier()?;
                if self.peek() == Some(&Token::LeftParen) {
                    self.next();
                    let mut arguments = Vec::new();
                    if self.peek() != Some(&Token::RightParen) {
                        loop {
                            arguments.push(self.parse_expression()?);
                            if self.peek() == Some(&Token::RightParen) {
                                break;
                            }
                            self.expect(Token::Comma)?;
                        }
                    }
                    self.expect(Token::RightParen)?;
                    Ok(Expression::Call(CallExpression {
                        function: name,
                        arguments,
                    }))
                } else {
                    Ok(Expression::Identifier(name))
                }
            }
            Some(Token::StringLiteral(_)) => {
                let value = self.parse_string()?;
                Ok(Expression::Literal(Literal::String(value)))
            }
            Some(Token::IntegerLiteral(_)) => {
                let value = self.parse_integer()?;
                Ok(Expression::Literal(Literal::Integer(value)))
            }
            Some(Token::FloatLiteral(_)) => {
                let value = self.parse_float()?;
                Ok(Expression::Literal(Literal::Float(value)))
            }
            Some(Token::LeftParen) => {
                self.next();
                let expr = self.parse_expression()?;
                self.expect(Token::RightParen)?;
                Ok(expr)
            }
            _ => Err(format!("Unexpected token: {:?}", self.peek())),
        }
    }

    fn parse_type(&mut self) -> Result<Type, String> {
        match self.peek() {
            Some(Token::Identifier(name)) => {
                self.next();
                Ok(Type::Named(name.clone()))
            }
            Some(Token::LeftBracket) => {
                self.next();
                let element_type = Box::new(self.parse_type()?);
                self.expect(Token::RightBracket)?;
                Ok(Type::Array(element_type))
            }
            _ => Err(format!("Unexpected token in type: {:?}", self.peek())),
        }
    }

    fn parse_type_definition(&mut self) -> Result<TypeDefinition, String> {
        self.expect(Token::Type)?;
        let name = self.parse_identifier()?;
        self.expect(Token::Equals)?;
        let type_ = self.parse_type()?;
        Ok(TypeDefinition { name, type_ })
    }

    fn parse_pattern(&mut self) -> Result<Pattern, String> {
        match self.peek() {
            Some(Token::Identifier(name)) => {
                self.next();
                Ok(Pattern::Identifier(name.clone()))
            }
            Some(Token::LeftParen) => {
                self.next();
                let mut patterns = Vec::new();
                if self.peek() != Some(&Token::RightParen) {
                    loop {
                        patterns.push(self.parse_pattern()?);
                        if self.peek() == Some(&Token::RightParen) {
                            break;
                        }
                        self.expect(Token::Comma)?;
                    }
                }
                self.expect(Token::RightParen)?;
                Ok(Pattern::Tuple(patterns))
            }
            _ => Err(format!("Unexpected token in pattern: {:?}", self.peek())),
        }
    }

    fn parse_identifier(&mut self) -> Result<String, String> {
        match self.peek() {
            Some(Token::Identifier(name)) => {
                self.next();
                Ok(name.clone())
            }
            _ => Err(format!("Expected identifier, got {:?}", self.peek())),
        }
    }

    fn parse_string(&mut self) -> Result<String, String> {
        match self.peek() {
            Some(Token::StringLiteral(value)) => {
                self.next();
                Ok(value.clone())
            }
            _ => Err(format!("Expected string literal, got {:?}", self.peek())),
        }
    }

    fn parse_integer(&mut self) -> Result<i64, String> {
        match self.peek() {
            Some(Token::IntegerLiteral(value)) => {
                self.next();
                Ok(*value)
            }
            _ => Err(format!("Expected integer literal, got {:?}", self.peek())),
        }
    }

    fn parse_float(&mut self) -> Result<f64, String> {
        match self.peek() {
            Some(Token::FloatLiteral(value)) => {
                self.next();
                Ok(*value)
            }
            _ => Err(format!("Expected float literal, got {:?}", self.peek())),
        }
    }

    fn expect(&mut self, token: Token) -> Result<(), String> {
        match self.peek() {
            Some(t) if t == &token => {
                self.next();
                Ok(())
            }
            _ => Err(format!("Expected {:?}, got {:?}", token, self.peek())),
        }
    }

    fn peek(&self) -> Option<&Token> {
        self.lexer.peek()
    }

    fn next(&mut self) -> Option<&Token> {
        self.lexer.next()
    }
} 