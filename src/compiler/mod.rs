use crate::ir::*;
use crate::parser::*;
use crate::lexer::*;
use crate::ast::*;

pub struct Compiler {
    ast: AST,
    ir: IR,
}

impl Compiler {
    pub fn new() -> Self {
        Compiler {
            ast: AST::new(),
            ir: IR::new(),
        }
    }

    pub fn compile(&mut self, source: &str) -> Result<IR, String> {
        // 字句解析
        let mut lexer = Lexer::new(source);
        let tokens = lexer.tokenize()?;

        // 構文解析
        let mut parser = Parser::new(&tokens);
        self.ast = parser.parse()?;

        // IR生成
        self.ir = self.generate_ir()?;

        Ok(self.ir.clone())
    }

    fn generate_ir(&self) -> Result<IR, String> {
        let mut ir = IR::new();
        
        // ASTをIRに変換
        for function in &self.ast.functions {
            let ir_function = self.convert_function(function)?;
            ir.add_function(ir_function);
        }

        Ok(ir)
    }

    fn convert_function(&self, function: &Function) -> Result<IRFunction, String> {
        let mut ir_function = IRFunction::new(
            function.name.clone(),
            function.return_type.clone(),
            function.priority.clone(),
        );

        // パラメータの変換
        for param in &function.parameters {
            ir_function.add_parameter(param.name.clone(), param.type_annotation.clone());
        }

        // 本体の変換
        for statement in &function.body {
            let ir_statement = self.convert_statement(statement)?;
            ir_function.add_statement(ir_statement);
        }

        Ok(ir_function)
    }

    fn convert_statement(&self, statement: &Statement) -> Result<IRStatement, String> {
        match statement {
            Statement::Let(let_stmt) => {
                let value = self.convert_expression(&let_stmt.value)?;
                Ok(IRStatement::Let {
                    name: let_stmt.name.clone(),
                    type_annotation: let_stmt.type_annotation.clone(),
                    value,
                })
            }
            Statement::Return(expr) => {
                let value = self.convert_expression(expr)?;
                Ok(IRStatement::Return(value))
            }
            Statement::Expression(expr) => {
                let value = self.convert_expression(expr)?;
                Ok(IRStatement::Expression(value))
            }
            // 他の文の変換も実装
            _ => Err("Unsupported statement type".to_string()),
        }
    }

    fn convert_expression(&self, expression: &Expression) -> Result<IRExpression, String> {
        match expression {
            Expression::Literal(lit) => Ok(IRExpression::Literal(lit.clone())),
            Expression::Identifier(name) => Ok(IRExpression::Identifier(name.clone())),
            Expression::BinaryOp { left, op, right } => {
                let left_ir = self.convert_expression(left)?;
                let right_ir = self.convert_expression(right)?;
                Ok(IRExpression::BinaryOp {
                    left: Box::new(left_ir),
                    op: op.clone(),
                    right: Box::new(right_ir),
                })
            }
            // 他の式の変換も実装
            _ => Err("Unsupported expression type".to_string()),
        }
    }
} 