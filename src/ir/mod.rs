use crate::type_system::Type;
use std::fmt;

#[derive(Debug, Clone)]
pub struct Module {
    pub functions: Vec<Function>,
    pub globals: Vec<Global>,
}

#[derive(Debug, Clone)]
pub struct Function {
    pub name: String,
    pub params: Vec<Parameter>,
    pub return_type: Type,
    pub blocks: Vec<Block>,
    pub priority: Option<FunctionPriority>,
}

#[derive(Debug, Clone)]
pub struct Parameter {
    pub name: String,
    pub type_: Type,
}

#[derive(Debug, Clone)]
pub struct Global {
    pub name: String,
    pub type_: Type,
    pub value: Constant,
    pub priority: Option<MemoryPriority>,
}

#[derive(Debug, Clone)]
pub struct Block {
    pub name: String,
    pub instructions: Vec<Instruction>,
    pub terminator: Terminator,
}

#[derive(Debug, Clone)]
pub enum Instruction {
    Alloca(AllocaInst),
    Load(LoadInst),
    Store(StoreInst),
    Binary(BinaryInst),
    Unary(UnaryInst),
    Call(CallInst),
    GetElementPtr(GetElementPtrInst),
    Phi(PhiInst),
    Priority(PriorityInst),
}

#[derive(Debug, Clone)]
pub struct AllocaInst {
    pub name: String,
    pub type_: Type,
}

#[derive(Debug, Clone)]
pub struct LoadInst {
    pub name: String,
    pub type_: Type,
    pub pointer: Value,
}

#[derive(Debug, Clone)]
pub struct StoreInst {
    pub value: Value,
    pub pointer: Value,
}

#[derive(Debug, Clone)]
pub struct BinaryInst {
    pub name: String,
    pub type_: Type,
    pub op: BinaryOp,
    pub left: Value,
    pub right: Value,
}

#[derive(Debug, Clone)]
pub enum BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Rem,
    Shl,
    Shr,
    And,
    Or,
    Xor,
}

#[derive(Debug, Clone)]
pub struct UnaryInst {
    pub name: String,
    pub type_: Type,
    pub op: UnaryOp,
    pub operand: Value,
}

#[derive(Debug, Clone)]
pub enum UnaryOp {
    Neg,
    Not,
}

#[derive(Debug, Clone)]
pub struct CallInst {
    pub name: String,
    pub type_: Type,
    pub callee: String,
    pub args: Vec<Value>,
}

#[derive(Debug, Clone)]
pub struct GetElementPtrInst {
    pub name: String,
    pub type_: Type,
    pub base: Value,
    pub indices: Vec<Value>,
}

#[derive(Debug, Clone)]
pub struct PhiInst {
    pub name: String,
    pub type_: Type,
    pub incoming: Vec<(Value, String)>,
}

#[derive(Debug, Clone)]
pub struct PriorityInst {
    pub name: String,
    pub type_: Type,
    pub priority_type: PriorityType,
    pub value: Value,
}

#[derive(Debug, Clone)]
pub enum Terminator {
    Ret(Option<Value>),
    Br(String),
    CondBr(Value, String, String),
    Switch(Value, String, Vec<(Constant, String)>),
}

#[derive(Debug, Clone)]
pub enum Value {
    Constant(Constant),
    Instruction(String),
    Parameter(String),
    Global(String),
}

#[derive(Debug, Clone)]
pub enum Constant {
    Int(i64),
    Float(f64),
    Bool(bool),
    Char(char),
    String(String),
    Array(Vec<Constant>),
    Struct(Vec<Constant>),
    Null,
}

#[derive(Debug, Clone)]
pub enum PriorityType {
    Log(LogPriority),
    Function(FunctionPriority),
    Memory(MemoryPriority),
}

#[derive(Debug, Clone)]
pub enum LogPriority {
    Log,
    Info,
    Debug,
    Warn,
    Alert,
    Error,
}

#[derive(Debug, Clone)]
pub enum FunctionPriority {
    Level(i32),
    MostLow,
    MostHigh,
}

#[derive(Debug, Clone)]
pub enum MemoryPriority {
    Level(i32),
    MultiLevel(Vec<i32>),
    MostLow,
    MostHigh,
}

impl fmt::Display for Module {
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