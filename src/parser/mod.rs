use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::lexer::{Lexer, Token};
use crate::type_system::Type;
use std::ops::Range;

pub struct Parser<'a> {
    lexer: &'a mut Lexer<'a>,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: &'a mut Lexer<'a>) -> Self {
        Self { lexer }
    }

    pub fn parse(&mut self) -> Result<AST> {
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
                _ => return Err(SlangError::Syntax(format!("Unexpected token: {:?}", token))),
            }
        }
        Ok(ast)
    }

    fn parse_function(&mut self) -> Result<Function> {
        self.expect(Token::Function)?;
        let name = self.parse_identifier()?;
        let parameters = self.parse_parameters()?;
        let return_type = self.parse_return_type()?;
        let priorities = self.parse_priority()?;
        let body = self.parse_block()?;
        Ok(Function {
            name,
            parameters,
            return_type,
            priorities,
            body,
        })
    }

    fn parse_parameters(&mut self) -> Result<Vec<Parameter>> {
        self.expect(Token::LParen)?;
        let mut parameters = Vec::new();
        if let Some(token) = self.peek() {
            if token != &Token::RParen {
                loop {
                    let name = self.parse_identifier()?;
                    self.expect(Token::Colon)?;
                    let type_ = self.parse_type()?;
                    parameters.push(Parameter { name, type_ });
                    if let Some(token) = self.peek() {
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
        Ok(parameters)
    }

    fn parse_return_type(&mut self) -> Result<Type> {
        self.expect(Token::Arrow)?;
        self.parse_type()
    }

    fn parse_priority(&mut self) -> Result<Vec<i32>> {
        let mut priorities = Vec::new();
        if let Some(token) = self.peek() {
            if token == &Token::Priority {
                self.next();
                self.expect(Token::LeftBracket)?;
                loop {
                    if let Some(token) = self.peek() {
                        match token {
                            Token::MostHigh => {
                                priorities.push(i32::MAX);
                                self.next();
                            }
                            Token::IntegerLiteral(priority) => {
                                priorities.push(*priority as i32);
                                self.next();
                            }
                            _ => break,
                        }
                        if let Some(token) = self.peek() {
                            if token == &Token::RightBracket {
                                break;
                            }
                            self.expect(Token::Comma)?;
                        } else {
                            return Err(SlangError::Syntax("Expected ']' or ','".to_string()));
                        }
                    } else {
                        return Err(SlangError::Syntax("Expected priority value".to_string()));
                    }
                }
                self.expect(Token::RightBracket)?;
            }
        }
        Ok(priorities)
    }

    fn parse_block(&mut self) -> Result<Block> {
        self.expect(Token::LeftBrace)?;
        let mut statements = Vec::new();
        while let Some(token) = self.peek() {
            if token == &Token::RightBrace {
                break;
            }
            statements.push(self.parse_statement()?);
        }
        self.expect(Token::RightBrace)?;
        Ok(Block { statements })
    }

    fn parse_statement(&mut self) -> Result<Statement> {
        match self.peek() {
            Some(Token::Let) => {
                self.next();
                let name = self.parse_identifier()?;
                self.expect(Token::Equals)?;
                let value = self.parse_expression()?;
                self.expect(Token::Semicolon)?;
                Ok(Statement::Let(LetStatement { name, value }))
            }
            Some(Token::Return) => {
                self.next();
                let value = self.parse_expression()?;
                self.expect(Token::Semicolon)?;
                Ok(Statement::Return(ReturnStatement { value }))
            }
            Some(Token::If) => {
                self.next();
                self.expect(Token::LParen)?;
                let condition = self.parse_expression()?;
                self.expect(Token::RParen)?;
                let then_branch = self.parse_block()?;
                let else_branch = if let Some(token) = self.peek() {
                    if token == &Token::Else {
                        self.next();
                        Some(self.parse_block()?)
                    } else {
                        None
                    }
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
                self.expect(Token::LParen)?;
                let condition = self.parse_expression()?;
                self.expect(Token::RParen)?;
                let body = self.parse_block()?;
                Ok(Statement::While(WhileStatement { condition, body }))
            }
            Some(Token::For) => {
                self.next();
                self.expect(Token::LParen)?;
                let pattern = self.parse_pattern()?;
                self.expect(Token::In)?;
                let iterable = self.parse_expression()?;
                self.expect(Token::RParen)?;
                let body = self.parse_block()?;
                Ok(Statement::For(ForStatement {
                    pattern,
                    iterable,
                    body,
                }))
            }
            Some(Token::Match) => {
                self.next();
                let value = self.parse_expression()?;
                self.expect(Token::LeftBrace)?;
                let mut arms = Vec::new();
                while let Some(token) = self.peek() {
                    if token == &Token::RightBrace {
                        break;
                    }
                    let pattern = self.parse_pattern()?;
                    self.expect(Token::FatArrow)?;
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

    fn parse_expression(&mut self) -> Result<Expression> {
        self.parse_assignment()
    }

    fn parse_assignment(&mut self) -> Result<Expression> {
        let expr = self.parse_equality()?;
        if let Some(token) = self.peek() {
            if token == &Token::Equals {
                self.next();
                let value = self.parse_assignment()?;
                if let Expression::Identifier(name) = expr {
                    return Ok(Expression::Assignment(AssignmentExpression { name, value }));
                }
                return Err(SlangError::Syntax("Invalid assignment target".to_string()));
            }
        }
        Ok(expr)
    }

    fn parse_equality(&mut self) -> Result<Expression> {
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

    fn parse_comparison(&mut self) -> Result<Expression> {
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

    fn parse_term(&mut self) -> Result<Expression> {
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
                        op: BinaryOperator::Sub,
                        right: Box::new(right),
                    });
                }
                _ => break,
            }
        }
        Ok(expr)
    }

    fn parse_factor(&mut self) -> Result<Expression> {
        let mut expr = self.parse_unary()?;
        while let Some(token) = self.peek() {
            match token {
                Token::Star => {
                    self.next();
                    let right = self.parse_unary()?;
                    expr = Expression::BinaryOp(BinaryOpExpression {
                        left: Box::new(expr),
                        op: BinaryOperator::Mul,
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

    fn parse_unary(&mut self) -> Result<Expression> {
        if let Some(token) = self.peek() {
            match token {
                Token::Minus => {
                    self.next();
                    let expr = self.parse_unary()?;
                    Ok(Expression::UnaryOp(UnaryOpExpression {
                        op: UnaryOperator::Neg,
                        expr: Box::new(expr),
                    }))
                }
                Token::Not => {
                    self.next();
                    let expr = self.parse_unary()?;
                    Ok(Expression::UnaryOp(UnaryOpExpression {
                        op: UnaryOperator::Not,
                        expr: Box::new(expr),
                    }))
                }
                _ => self.parse_primary(),
            }
        } else {
            Err(SlangError::Syntax("Expected expression".to_string()))
        }
    }

    fn parse_primary(&mut self) -> Result<Expression> {
        match self.peek() {
            Some(Token::Identifier(name)) => {
                self.next();
                if let Some(token) = self.peek() {
                    if token == &Token::LParen {
                        self.next();
                        let mut arguments = Vec::new();
                        if let Some(token) = self.peek() {
                            if token != &Token::RParen {
                                loop {
                                    arguments.push(self.parse_expression()?);
                                    if let Some(token) = self.peek() {
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
                        Ok(Expression::Call(CallExpression {
                            function: name.clone(),
                            arguments,
                        }))
                    } else {
                        Ok(Expression::Identifier(name.clone()))
                    }
                } else {
                    Ok(Expression::Identifier(name.clone()))
                }
            }
            Some(Token::StringLiteral(value)) => {
                self.next();
                Ok(Expression::Literal(Literal::String(value.clone())))
            }
            Some(Token::IntegerLiteral(value)) => {
                self.next();
                Ok(Expression::Literal(Literal::Int(*value)))
            }
            Some(Token::FloatLiteral(value)) => {
                self.next();
                Ok(Expression::Literal(Literal::Float(*value)))
            }
            Some(Token::LParen) => {
                self.next();
                let expr = self.parse_expression()?;
                self.expect(Token::RParen)?;
                Ok(expr)
            }
            _ => Err(SlangError::Syntax(format!("Unexpected token: {:?}", self.peek()))),
        }
    }

    fn parse_type(&mut self) -> Result<Type> {
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
            Some(Token::LParen) => {
                self.next();
                let mut types = Vec::new();
                if let Some(token) = self.peek() {
                    if token != &Token::RParen {
                        loop {
                            types.push(self.parse_type()?);
                            if let Some(token) = self.peek() {
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
            _ => Err(SlangError::Syntax(format!("Unexpected token in type: {:?}", self.peek()))),
        }
    }

    fn parse_type_definition(&mut self) -> Result<TypeDefinition> {
        self.expect(Token::Type)?;
        let name = self.parse_identifier()?;
        self.expect(Token::Equals)?;
        let type_ = self.parse_type()?;
        self.expect(Token::Semicolon)?;
        Ok(TypeDefinition { name, type_ })
    }

    fn parse_pattern(&mut self) -> Result<Pattern> {
        match self.peek() {
            Some(Token::Identifier(name)) => {
                self.next();
                Ok(Pattern::Identifier(name.clone()))
            }
            Some(Token::Underscore) => {
                self.next();
                Ok(Pattern::Wildcard)
            }
            Some(Token::LParen) => {
                self.next();
                let mut patterns = Vec::new();
                if let Some(token) = self.peek() {
                    if token != &Token::RParen {
                        loop {
                            patterns.push(self.parse_pattern()?);
                            if let Some(token) = self.peek() {
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
            _ => Err(SlangError::Syntax(format!("Unexpected token in pattern: {:?}", self.peek()))),
        }
    }

    fn parse_identifier(&mut self) -> Result<String> {
        match self.peek() {
            Some(Token::Identifier(name)) => {
                self.next();
                Ok(name.clone())
            }
            _ => Err(SlangError::Syntax(format!("Expected identifier, got {:?}", self.peek()))),
        }
    }

    fn parse_string(&mut self) -> Result<String> {
        match self.peek() {
            Some(Token::StringLiteral(value)) => {
                self.next();
                Ok(value.clone())
            }
            _ => Err(SlangError::Syntax(format!("Expected string literal, got {:?}", self.peek()))),
        }
    }

    fn parse_integer(&mut self) -> Result<i64> {
        match self.peek() {
            Some(Token::IntegerLiteral(value)) => {
                self.next();
                Ok(*value)
            }
            _ => Err(SlangError::Syntax(format!("Expected integer literal, got {:?}", self.peek()))),
        }
    }

    fn parse_float(&mut self) -> Result<f64> {
        match self.peek() {
            Some(Token::FloatLiteral(value)) => {
                self.next();
                Ok(*value)
            }
            _ => Err(SlangError::Syntax(format!("Expected float literal, got {:?}", self.peek()))),
        }
    }

    fn expect(&mut self, token: Token) -> Result<()> {
        match self.peek() {
            Some(t) if t == &token => {
                self.next();
                Ok(())
            }
            _ => Err(SlangError::Syntax(format!("Expected {:?}, got {:?}", token, self.peek()))),
        }
    }

    fn peek(&self) -> Option<&Token> {
        self.lexer.peek()
    }

    fn next(&mut self) -> Option<&Token> {
        self.lexer.next()
    }
} 