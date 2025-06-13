use crate::type_system::Type;
use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub struct IR {
    pub functions: Vec<IRFunction>,
    pub globals: Vec<IRGlobal>,
}

impl IR {
    pub fn new() -> Self {
        Self {
            functions: Vec::new(),
            globals: Vec::new(),
        }
    }

    pub fn add_function(&mut self, func: IRFunction) {
        self.functions.push(func);
    }

    pub fn add_global(&mut self, global: IRGlobal) {
        self.globals.push(global);
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct IRFunction {
    pub name: String,
    pub parameters: Vec<IRParameter>,
    pub return_type: Type,
    pub priority: i32,
    pub blocks: Vec<IRBlock>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct IRParameter {
    pub name: String,
    pub type_annotation: Type,
}

#[derive(Debug, Clone, PartialEq)]
pub struct IRGlobal {
    pub name: String,
    pub type_annotation: Type,
    pub value: IRValue,
}

#[derive(Debug, Clone, PartialEq)]
pub struct IRBlock {
    pub label: String,
    pub instructions: Vec<IRInstruction>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum IRInstruction {
    Alloca { name: String, type_annotation: Type },
    Store { name: String, value: IRValue },
    Load { name: String },
    BinaryOp { dest: String, op: IRBinaryOperator, left: IRValue, right: IRValue },
    UnaryOp { dest: String, op: IRUnaryOperator, expr: IRValue },
    Call { dest: String, function: String, arguments: Vec<IRValue> },
    Return(Option<IRValue>),
    Branch { label: String },
    ConditionalBranch { condition: IRValue, then_label: String, else_label: String },
    Assignment { target: String, value: IRValue },
    Expression(IRValue),
}

#[derive(Debug, Clone, PartialEq)]
pub enum IRBinaryOperator {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Eq,
    Neq,
    Lt,
    Lte,
    Gt,
    Gte,
    And,
    Or,
}

#[derive(Debug, Clone, PartialEq)]
pub enum IRUnaryOperator {
    Neg,
    Not,
}

#[derive(Debug, Clone, PartialEq)]
pub enum IRValue {
    Int(i64),
    Float(f64),
    Bool(bool),
    String(String),
    Null,
    Identifier(String),
    // 変数や関数呼び出しなどの値
}

impl fmt::Display for IR {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for func in &self.functions {
            writeln!(f, "{}", func)?;
        }
        for global in &self.globals {
            writeln!(f, "{}", global)?;
        }
        Ok(())
    }
}

impl fmt::Display for IRFunction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "fn {}({}) -> {} [priority: {}] {{\n", self.name, self.parameters.iter().map(|p| p.to_string()).collect::<Vec<_>>().join(", "), self.return_type, self.priority)?;
        for block in &self.blocks {
            writeln!(f, "  {}", block)?;
        }
        write!(f, "}}")
    }
}

impl fmt::Display for IRParameter {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}: {}", self.name, self.type_annotation)
    }
}

impl fmt::Display for IRBlock {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}: ", self.label)?;
        for instr in &self.instructions {
            writeln!(f, "    {}", instr)?;
        }
        Ok(())
    }
}

impl fmt::Display for IRInstruction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRInstruction::Alloca { name, type_annotation } => write!(f, "alloca {}: {}", name, type_annotation),
            IRInstruction::Store { name, value } => write!(f, "store {}, {}", name, value),
            IRInstruction::Load { name } => write!(f, "load {}", name),
            IRInstruction::BinaryOp { dest, op, left, right } => write!(f, "{} = {} {} {}", dest, left, op, right),
            IRInstruction::UnaryOp { dest, op, expr } => write!(f, "{} = {} {}", dest, op, expr),
            IRInstruction::Call { dest, function, arguments } => write!(f, "{} = call {}({})", dest, function, arguments.iter().map(|a| a.to_string()).collect::<Vec<_>>().join(", ")),
            IRInstruction::Return(Some(value)) => write!(f, "return {}", value),
            IRInstruction::Return(None) => write!(f, "return"),
            IRInstruction::Branch { label } => write!(f, "br {}", label),
            IRInstruction::ConditionalBranch { condition, then_label, else_label } => write!(f, "br_if {} {} {}", condition, then_label, else_label),
            IRInstruction::Assignment { target, value } => write!(f, "{} = {}", target, value),
            IRInstruction::Expression(value) => write!(f, "expr {}", value),
        }
    }
}

impl fmt::Display for IRValue {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRValue::Int(i) => write!(f, "{}", i),
            IRValue::Float(fl) => write!(f, "{}", fl),
            IRValue::Bool(b) => write!(f, "{}", b),
            IRValue::String(s) => write!(f, "\"{}\"", s),
            IRValue::Null => write!(f, "null"),
            IRValue::Identifier(name) => write!(f, "{}", name),
        }
    }
}

impl fmt::Display for IRBinaryOperator {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use IRBinaryOperator::*;
        write!(f, "{}", match self {
            Add => "+",
            Sub => "-",
            Mul => "*",
            Div => "/",
            Mod => "%",
            Eq => "==",
            Neq => "!=",
            Lt => "<",
            Lte => "<=",
            Gt => ">",
            Gte => ">=",
            And => "&&",
            Or => "||",
        })
    }
}

impl fmt::Display for IRUnaryOperator {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use IRUnaryOperator::*;
        write!(f, "{}", match self {
            Neg => "-",
            Not => "!",
        })
    }
} 