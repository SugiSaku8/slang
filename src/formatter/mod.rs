use crate::ast::*;
use crate::error::Result;
use std::fmt::Write;

pub struct Formatter {
    indent_level: usize,
    indent_size: usize,
    line_length: usize,
}

impl Formatter {
    pub fn new() -> Self {
        Self {
            indent_level: 0,
            indent_size: 4,
            line_length: 100,
        }
    }

    pub fn format(&mut self, program: &Program) -> Result<String> {
        let mut output = String::new();
        self.format_program(program, &mut output)?;
        Ok(output)
    }

    fn format_program(&mut self, program: &Program, output: &mut String) -> Result<()> {
        for (i, item) in program.items.iter().enumerate() {
            if i > 0 {
                writeln!(output)?;
            }
            self.format_item(item, output)?;
        }
        Ok(())
    }

    fn format_item(&mut self, item: &Item, output: &mut String) -> Result<()> {
        match item {
            Item::Function(func) => self.format_function(func, output)?,
            Item::Struct(struct_) => self.format_struct(struct_, output)?,
            Item::Trait(trait_) => self.format_trait(trait_, output)?,
            Item::Module(module) => self.format_module(module, output)?,
            Item::Macro(macro_) => self.format_macro(macro_, output)?,
        }
        Ok(())
    }

    fn format_function(&mut self, func: &Function, output: &mut String) -> Result<()> {
        // 関数の優先所有格
        if let Some(priority) = &func.priority {
            writeln!(output, "Function:type:priority:{:?}", priority)?;
        }

        // 関数の定義
        write!(output, "fn {}(", func.name)?;
        self.format_parameters(&func.params, output)?;
        write!(output, ") -> {}", func.return_type)?;
        writeln!(output)?;

        // 関数の本体
        self.format_block(&func.body, output)?;
        Ok(())
    }

    fn format_parameters(&self, params: &[Parameter], output: &mut String) -> Result<()> {
        for (i, param) in params.iter().enumerate() {
            if i > 0 {
                write!(output, ", ")?;
            }
            write!(output, "{}: {}", param.name, param.type_)?;
        }
        Ok(())
    }

    fn format_block(&mut self, block: &Block, output: &mut String) -> Result<()> {
        writeln!(output, "{{")?;
        self.indent_level += 1;

        for statement in &block.statements {
            self.format_statement(statement, output)?;
        }

        self.indent_level -= 1;
        writeln!(output, "}}")?;
        Ok(())
    }

    fn format_statement(&mut self, statement: &Statement, output: &mut String) -> Result<()> {
        self.write_indent(output)?;
        match statement {
            Statement::Let(let_stmt) => self.format_let_statement(let_stmt, output)?,
            Statement::Expression(expr) => {
                self.format_expression(expr, output)?;
                writeln!(output, ";")?;
            }
            Statement::Return(expr) => {
                write!(output, "return")?;
                if let Some(expr) = expr {
                    write!(output, " ")?;
                    self.format_expression(expr, output)?;
                }
                writeln!(output, ";")?;
            }
            Statement::If(if_stmt) => self.format_if_statement(if_stmt, output)?,
            Statement::Match(match_stmt) => self.format_match_statement(match_stmt, output)?,
            Statement::While(while_stmt) => self.format_while_statement(while_stmt, output)?,
            Statement::For(for_stmt) => self.format_for_statement(for_stmt, output)?,
            Statement::Priority(priority_stmt) => self.format_priority_statement(priority_stmt, output)?,
        }
        Ok(())
    }

    fn format_let_statement(&mut self, let_stmt: &LetStatement, output: &mut String) -> Result<()> {
        // 変数の優先所有格
        if let Some(priority) = &let_stmt.priority {
            writeln!(output, "Var:type:priority:{:?}", priority)?;
            self.write_indent(output)?;
        }

        write!(output, "let {}", let_stmt.name)?;
        if let Some(type_) = &let_stmt.type_ {
            write!(output, ": {}", type_)?;
        }
        write!(output, " = ")?;
        self.format_expression(&let_stmt.value, output)?;
        writeln!(output, ";")?;
        Ok(())
    }

    fn format_expression(&self, expr: &Expression, output: &mut String) -> Result<()> {
        match expr {
            Expression::Literal(lit) => self.format_literal(lit, output)?,
            Expression::Identifier(name) => write!(output, "{}", name)?,
            Expression::Binary(bin) => {
                self.format_expression(&bin.left, output)?;
                write!(output, " {} ", self.format_binary_operator(&bin.operator))?;
                self.format_expression(&bin.right, output)?;
            }
            Expression::Unary(unary) => {
                write!(output, "{}", self.format_unary_operator(&unary.operator))?;
                self.format_expression(&unary.expression, output)?;
            }
            Expression::Call(call) => {
                self.format_expression(&call.callee, output)?;
                write!(output, "(")?;
                for (i, arg) in call.arguments.iter().enumerate() {
                    if i > 0 {
                        write!(output, ", ")?;
                    }
                    self.format_expression(arg, output)?;
                }
                write!(output, ")")?;
            }
            // 他の式のフォーマットも同様に実装...
            _ => unimplemented!(),
        }
        Ok(())
    }

    fn format_literal(&self, lit: &Literal, output: &mut String) -> Result<()> {
        match lit {
            Literal::Integer(value) => write!(output, "{}", value)?,
            Literal::Float(value) => write!(output, "{}", value)?,
            Literal::Boolean(value) => write!(output, "{}", value)?,
            Literal::Character(value) => write!(output, "'{}'", value)?,
            Literal::String(value) => write!(output, "\"{}\"", value)?,
            Literal::Array(elements) => {
                write!(output, "[")?;
                for (i, element) in elements.iter().enumerate() {
                    if i > 0 {
                        write!(output, ", ")?;
                    }
                    self.format_expression(element, output)?;
                }
                write!(output, "]")?;
            }
            Literal::Tuple(elements) => {
                write!(output, "(")?;
                for (i, element) in elements.iter().enumerate() {
                    if i > 0 {
                        write!(output, ", ")?;
                    }
                    self.format_expression(element, output)?;
                }
                write!(output, ")")?;
            }
        }
        Ok(())
    }

    fn format_binary_operator(&self, op: &BinaryOperator) -> &'static str {
        match op {
            BinaryOperator::Add => "+",
            BinaryOperator::Subtract => "-",
            BinaryOperator::Multiply => "*",
            BinaryOperator::Divide => "/",
            BinaryOperator::Modulo => "%",
            BinaryOperator::Equal => "==",
            BinaryOperator::NotEqual => "!=",
            BinaryOperator::LessThan => "<",
            BinaryOperator::GreaterThan => ">",
            BinaryOperator::LessEqual => "<=",
            BinaryOperator::GreaterEqual => ">=",
            BinaryOperator::And => "&&",
            BinaryOperator::Or => "||",
        }
    }

    fn format_unary_operator(&self, op: &UnaryOperator) -> &'static str {
        match op {
            UnaryOperator::Negate => "-",
            UnaryOperator::Not => "!",
        }
    }

    fn write_indent(&self, output: &mut String) -> Result<()> {
        write!(output, "{}", " ".repeat(self.indent_level * self.indent_size))?;
        Ok(())
    }
} 