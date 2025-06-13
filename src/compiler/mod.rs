use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::ir::*;
use crate::lexer::Lexer;
use crate::parser::Parser;

pub struct Compiler {
    ast: AST,
    ir: IR,
}

impl Compiler {
    pub fn new() -> Self {
        Self {
            ast: AST::new(),
            ir: IR::new(),
        }
    }

    pub fn compile(&mut self, source: &str) -> Result<IR> {
        let mut lexer = Lexer::new(source);
        let tokens = lexer.tokenize()?;
        let mut parser = Parser::new(&mut lexer);
        self.ast = parser.parse()?;
        self.generate_ir()?;
        Ok(self.ir.clone())
    }

    fn generate_ir(&mut self) -> Result<()> {
        for function in &self.ast.functions {
            let ir_function = self.convert_function(function)?;
            self.ir.add_function(ir_function);
        }
        Ok(())
    }

    fn convert_function(&self, function: &Function) -> Result<IRFunction> {
        let mut ir_function = IRFunction {
            name: function.name.clone(),
            parameters: function
                .parameters
                .iter()
                .map(|p| IRParameter {
                    name: p.name.clone(),
                    type_: p.type_.clone(),
                })
                .collect(),
            return_type: function.return_type.clone(),
            blocks: vec![IRBlock {
                name: "entry".to_string(),
                instructions: Vec::new(),
            }],
        };

        for statement in &function.body {
            let instruction = self.convert_statement(statement)?;
            ir_function.blocks[0].instructions.push(instruction);
        }

        Ok(ir_function)
    }

    fn convert_statement(&self, statement: &Statement) -> Result<IRInstruction> {
        match statement {
            Statement::Let(LetStatement { name, value }) => {
                let value = self.convert_expression(value)?;
                Ok(IRInstruction::Let {
                    name: name.clone(),
                    value,
                })
            }
            Statement::Return(ReturnStatement { value }) => {
                let value = self.convert_expression(value)?;
                Ok(IRInstruction::Return(value))
            }
            Statement::Expression(expr) => {
                let value = self.convert_expression(expr)?;
                Ok(IRInstruction::Expression(value))
            }
            _ => Err(SlangError::Compilation("Unsupported statement".to_string())),
        }
    }

    fn convert_expression(&self, expression: &Expression) -> Result<IRValue> {
        match expression {
            Expression::Literal(lit) => Ok(IRValue::Constant(match lit {
                Literal::Integer(i) => IRConstant::Integer(*i),
                Literal::Float(f) => IRConstant::Float(*f),
                Literal::String(s) => IRConstant::String(s.clone()),
                Literal::Boolean(b) => IRConstant::Boolean(*b),
            })),
            Expression::Identifier(name) => Ok(IRValue::Variable(name.clone())),
            Expression::BinaryOp(BinaryOpExpression { left, op, right }) => {
                let left = self.convert_expression(left)?;
                let right = self.convert_expression(right)?;
                Ok(IRValue::BinaryOp {
                    left: Box::new(left),
                    op: match op {
                        BinaryOperator::Add => IRBinaryOperator::Add,
                        BinaryOperator::Subtract => IRBinaryOperator::Subtract,
                        BinaryOperator::Multiply => IRBinaryOperator::Multiply,
                        BinaryOperator::Divide => IRBinaryOperator::Divide,
                        BinaryOperator::Modulo => IRBinaryOperator::Modulo,
                        BinaryOperator::Equals => IRBinaryOperator::Equals,
                        BinaryOperator::NotEquals => IRBinaryOperator::NotEquals,
                        BinaryOperator::LessThan => IRBinaryOperator::LessThan,
                        BinaryOperator::GreaterThan => IRBinaryOperator::GreaterThan,
                        BinaryOperator::LessThanEquals => IRBinaryOperator::LessThanEquals,
                        BinaryOperator::GreaterThanEquals => IRBinaryOperator::GreaterThanEquals,
                    },
                    right: Box::new(right),
                })
            }
            Expression::UnaryOp(UnaryOpExpression { op, expr }) => {
                let expr = self.convert_expression(expr)?;
                Ok(IRValue::UnaryOp {
                    op: match op {
                        UnaryOperator::Negate => IRUnaryOperator::Negate,
                        UnaryOperator::Not => IRUnaryOperator::Not,
                    },
                    expr: Box::new(expr),
                })
            }
            Expression::Call(CallExpression { function, arguments }) => {
                let args = arguments
                    .iter()
                    .map(|arg| self.convert_expression(arg))
                    .collect::<Result<Vec<_>>>()?;
                Ok(IRValue::Call {
                    function: function.clone(),
                    arguments: args,
                })
            }
            Expression::Assignment(AssignmentExpression { name, value }) => {
                let value = self.convert_expression(value)?;
                Ok(IRValue::Assignment {
                    name: name.clone(),
                    value: Box::new(value),
                })
            }
        }
    }
} 