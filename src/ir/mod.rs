use crate::type_system::Type;
use std::fmt;

#[derive(Debug, Clone)]
pub struct IR {
    pub functions: Vec<IRFunction>,
    pub globals: Vec<IRGlobal>,
}

impl IR {
    pub fn new() -> Self {
        IR {
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
    pub priority: Vec<i32>,
    pub blocks: Vec<IRBlock>,
}

#[derive(Debug, Clone)]
pub struct IRParameter {
    pub name: String,
    pub type_: Type,
}

#[derive(Debug, Clone)]
pub struct IRGlobal {
    pub name: String,
    pub type_: Type,
    pub value: Option<IRConstant>,
}

#[derive(Debug, Clone)]
pub struct IRBlock {
    pub label: String,
    pub instructions: Vec<IRInstruction>,
}

#[derive(Debug, Clone)]
pub enum IRInstruction {
    Alloca(IRAlloca),
    Store(IRStore),
    Load(IRLoad),
    BinaryOp(IRBinaryOp),
    UnaryOp(IRUnaryOp),
    Call(IRCall),
    Return(Option<IRValue>),
    Branch(IRBranch),
    ConditionalBranch(IRConditionalBranch),
}

#[derive(Debug, Clone)]
pub struct IRAlloca {
    pub result: String,
    pub type_: Type,
}

#[derive(Debug, Clone)]
pub struct IRStore {
    pub value: IRValue,
    pub pointer: String,
}

#[derive(Debug, Clone)]
pub struct IRLoad {
    pub result: String,
    pub pointer: String,
    pub type_: Type,
}

#[derive(Debug, Clone)]
pub struct IRBinaryOp {
    pub result: String,
    pub op: IRBinaryOperator,
    pub left: IRValue,
    pub right: IRValue,
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
}

#[derive(Debug, Clone)]
pub struct IRUnaryOp {
    pub result: String,
    pub op: IRUnaryOperator,
    pub operand: IRValue,
}

#[derive(Debug, Clone)]
pub enum IRUnaryOperator {
    Negate,
    Not,
}

#[derive(Debug, Clone)]
pub struct IRCall {
    pub result: Option<String>,
    pub function: String,
    pub arguments: Vec<IRValue>,
}

#[derive(Debug, Clone)]
pub struct IRBranch {
    pub target: String,
}

#[derive(Debug, Clone)]
pub struct IRConditionalBranch {
    pub condition: IRValue,
    pub true_target: String,
    pub false_target: String,
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
    Boolean(bool),
    String(String),
    Array(Vec<IRConstant>),
    Tuple(Vec<IRConstant>),
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

// 他のDisplay実装も同様に実装... 