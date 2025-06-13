use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::ir::*;
use crate::lexer::Lexer;
use crate::parser::Parser;

pub struct Compiler {
    ast: AST,
}

impl Compiler {
    pub fn new() -> Self {
        Self {
            ast: AST::new(),
        }
    }

    pub fn compile(&mut self, source: &str) -> Result<IRFunction> {
        let lexer = Lexer::new(source);
        let mut parser = Parser::new(lexer);
        let ast = parser.parse()?;
        self.ast = ast;

        // 最初の関数をコンパイル
        if let Some(function) = self.ast.functions.first() {
            self.compile_function(function)
        } else {
            Err(SlangError::Compilation("No functions found".to_string()))
        }
    }

    fn compile_function(&self, function: &Function) -> Result<IRFunction> {
        let mut ir_function = IRFunction {
            name: function.name.clone(),
            parameters: function.parameters
                .iter()
                .map(|p| IRParameter {
                    name: p.name.clone(),
                    type_annotation: p.type_annotation.clone(),
                })
                .collect(),
            return_type: function.return_type.clone(),
            blocks: vec![IRBlock {
                label: "entry".to_string(),
                instructions: Vec::new(),
            }],
            priority: function.priority,
        };

        // 関数本体をコンパイル
        for statement in &function.body.statements {
            match statement {
                Statement::Let(LetStatement { name, value, type_annotation: _ }) => {
                    let value = self.convert_expression(value)?;
                    ir_function.blocks[0].instructions.push(IRInstruction::Store {
                        name: name.clone(),
                        value,
                    });
                }
                Statement::Return(ReturnStatement { value }) => {
                    if let Some(value) = value {
                        let value = self.convert_expression(value)?;
                        ir_function.blocks[0].instructions.push(IRInstruction::Return(Some(value)));
                    } else {
                        ir_function.blocks[0].instructions.push(IRInstruction::Return(None));
                    }
                }
                _ => return Err(SlangError::Compilation("Unsupported statement".to_string())),
            }
        }

        Ok(ir_function)
    }

    fn convert_expression(&self, expression: &Expression) -> Result<IRValue> {
        match expression {
            Expression::Literal(lit) => {
                match lit {
                    Literal::Int(i) => Ok(IRValue::Constant(IRConstant::Integer(*i))),
                    Literal::Float(f) => Ok(IRValue::Constant(IRConstant::Float(*f))),
                    Literal::String(s) => Ok(IRValue::Constant(IRConstant::String(s.clone()))),
                    Literal::Bool(b) => Ok(IRValue::Constant(IRConstant::Boolean(*b))),
                    Literal::Null => Ok(IRValue::Constant(IRConstant::Unit)),
                }
            }
            Expression::BinaryOp(BinaryOpExpression { left, op, right }) => {
                let left = self.convert_expression(left)?;
                let right = self.convert_expression(right)?;
                let op = match op {
                    BinaryOperator::Add => IRBinaryOperator::Add,
                    BinaryOperator::Sub => IRBinaryOperator::Sub,
                    BinaryOperator::Mul => IRBinaryOperator::Mul,
                    BinaryOperator::Div => IRBinaryOperator::Div,
                    BinaryOperator::Eq => IRBinaryOperator::Eq,
                    BinaryOperator::Neq => IRBinaryOperator::Neq,
                    BinaryOperator::Lt => IRBinaryOperator::Lt,
                    BinaryOperator::Lte => IRBinaryOperator::Lte,
                    BinaryOperator::Gt => IRBinaryOperator::Gt,
                    BinaryOperator::Gte => IRBinaryOperator::Gte,
                    BinaryOperator::And => IRBinaryOperator::And,
                    BinaryOperator::Or => IRBinaryOperator::Or,
                };
                Ok(IRValue::BinaryOp {
                    left: Box::new(left),
                    op,
                    right: Box::new(right),
                })
            }
            Expression::UnaryOp(UnaryOpExpression { op, right }) => {
                let expr = self.convert_expression(right)?;
                let op = match op {
                    UnaryOperator::Not => IRUnaryOperator::Not,
                    UnaryOperator::Neg => IRUnaryOperator::Neg,
                };
                Ok(IRValue::UnaryOp {
                    op,
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
            Expression::Assignment(AssignmentExpression { target, value }) => {
                let value = self.convert_expression(value)?;
                Ok(IRValue::Assignment {
                    target: target.clone(),
                    value: Box::new(value),
                })
            }
            Expression::Identifier(name) => {
                Ok(IRValue::Variable(name.clone()))
            }
        }
    }
} 