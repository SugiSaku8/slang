use crate::type_system::Type;
use std::fmt;

#[derive(Debug, Clone)]
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

    pub fn add_function(&mut self, function: IRFunction) {
        self.functions.push(function);
    }

    pub fn add_global(&mut self, global: IRGlobal) {
        self.globals.push(global);
    }
}

#[derive(Debug, Clone)]
pub struct IRFunction {
    pub name: String,
    pub parameters: Vec<IRParameter>,
    pub return_type: Type,
    pub priority: i32,
    pub blocks: Vec<IRBlock>,
}

#[derive(Debug, Clone)]
pub struct IRParameter {
    pub name: String,
    pub type_annotation: Type,
}

#[derive(Debug, Clone)]
pub struct IRGlobal {
    pub name: String,
    pub type_annotation: Type,
    pub value: IRValue,
}

#[derive(Debug, Clone)]
pub struct IRBlock {
    pub label: String,
    pub instructions: Vec<IRInstruction>,
}

#[derive(Debug, Clone)]
pub enum IRInstruction {
    Alloca(String, Type),
    Store(String, IRValue),
    Load(String, String),
    BinaryOp(String, IRBinaryOperator, IRValue, IRValue),
    UnaryOp(String, IRUnaryOperator, IRValue),
    Call(String, String, Vec<IRValue>),
    Return(Option<IRValue>),
    Branch(String),
    ConditionalBranch(IRValue, String, String),
}

#[derive(Debug, Clone)]
pub enum IRBinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Equals,
    NotEquals,
    LessThan,
    GreaterThan,
    LessThanEquals,
    GreaterThanEquals,
    And,
    Or,
}

#[derive(Debug, Clone)]
pub enum IRUnaryOperator {
    Negate,
    Not,
}

#[derive(Debug, Clone)]
pub enum IRValue {
    Constant(IRConstant),
    Variable(String),
}

#[derive(Debug, Clone)]
pub enum IRConstant {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
    Null,
}

impl fmt::Display for IR {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for global in &self.globals {
            writeln!(f, "{}", global)?;
        }
        for function in &self.functions {
            writeln!(f, "{}", function)?;
        }
        Ok(())
    }
}

impl fmt::Display for IRFunction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "fn {}(", self.name)?;
        for (i, param) in self.parameters.iter().enumerate() {
            if i > 0 {
                write!(f, ", ")?;
            }
            write!(f, "{}", param)?;
        }
        writeln!(f, ") -> {} {{", self.return_type)?;
        for block in &self.blocks {
            writeln!(f, "{}", block)?;
        }
        writeln!(f, "}}")
    }
}

impl fmt::Display for IRParameter {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}: {}", self.name, self.type_annotation)
    }
}

impl fmt::Display for IRGlobal {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "global {}: {} = {}", self.name, self.type_annotation, self.value)
    }
}

impl fmt::Display for IRBlock {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "{}:", self.label)?;
        for instruction in &self.instructions {
            writeln!(f, "    {}", instruction)?;
        }
        Ok(())
    }
}

impl fmt::Display for IRInstruction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRInstruction::Alloca(name, type_) => {
                writeln!(f, "{} = alloca {}", name, type_)
            }
            IRInstruction::Store(name, value) => {
                writeln!(f, "store {} = {}", name, value)
            }
            IRInstruction::Load(dest, src) => {
                writeln!(f, "{} = load {}", dest, src)
            }
            IRInstruction::BinaryOp(dest, op, left, right) => {
                writeln!(f, "{} = {} {} {}", dest, op, left, right)
            }
            IRInstruction::UnaryOp(dest, op, expr) => {
                writeln!(f, "{} = {} {}", dest, op, expr)
            }
            IRInstruction::Call(dest, function, args) => {
                write!(f, "{} = call {}(", dest, function)?;
                for (i, arg) in args.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", arg)?;
                }
                writeln!(f, ")")
            }
            IRInstruction::Return(value) => {
                if let Some(value) = value {
                    writeln!(f, "return {}", value)
                } else {
                    writeln!(f, "return")
                }
            }
            IRInstruction::Branch(label) => {
                writeln!(f, "br {}", label)
            }
            IRInstruction::ConditionalBranch(cond, true_label, false_label) => {
                writeln!(f, "br {} {}, {}", cond, true_label, false_label)
            }
        }
    }
}

impl fmt::Display for IRBinaryOperator {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRBinaryOperator::Add => write!(f, "add"),
            IRBinaryOperator::Subtract => write!(f, "sub"),
            IRBinaryOperator::Multiply => write!(f, "mul"),
            IRBinaryOperator::Divide => write!(f, "div"),
            IRBinaryOperator::Modulo => write!(f, "mod"),
            IRBinaryOperator::Equals => write!(f, "eq"),
            IRBinaryOperator::NotEquals => write!(f, "ne"),
            IRBinaryOperator::LessThan => write!(f, "lt"),
            IRBinaryOperator::GreaterThan => write!(f, "gt"),
            IRBinaryOperator::LessThanEquals => write!(f, "le"),
            IRBinaryOperator::GreaterThanEquals => write!(f, "ge"),
            IRBinaryOperator::And => write!(f, "and"),
            IRBinaryOperator::Or => write!(f, "or"),
        }
    }
}

impl fmt::Display for IRUnaryOperator {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRUnaryOperator::Negate => write!(f, "neg"),
            IRUnaryOperator::Not => write!(f, "not"),
        }
    }
}

impl fmt::Display for IRValue {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRValue::Constant(constant) => write!(f, "{}", constant),
            IRValue::Variable(name) => write!(f, "{}", name),
        }
    }
}

impl fmt::Display for IRConstant {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRConstant::Integer(i) => write!(f, "{}", i),
            IRConstant::Float(fl) => write!(f, "{}", fl),
            IRConstant::String(s) => write!(f, "\"{}\"", s),
            IRConstant::Boolean(b) => write!(f, "{}", b),
            IRConstant::Null => write!(f, "null"),
        }
    }
} 