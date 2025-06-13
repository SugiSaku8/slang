use std::collections::HashMap;
use crate::types::Type;

#[derive(Debug, Clone)]
pub enum IRValue {
    Integer(i64),
    Float(f64),
    Boolean(bool),
    String(String),
    Array(Vec<IRValue>),
    Struct(String, Vec<(String, IRValue)>),
    Function(String, Vec<String>, Box<IRBlock>),
    Null,
}

impl IRValue {
    pub fn get_type(&self) -> Type {
        match self {
            IRValue::Integer(_) => Type::Integer,
            IRValue::Float(_) => Type::Float,
            IRValue::Boolean(_) => Type::Boolean,
            IRValue::String(_) => Type::String,
            IRValue::Array(_) => Type::Array(Box::new(Type::Any)),
            IRValue::Struct(name, _) => Type::Struct(name.clone()),
            IRValue::Function(_, _, _) => Type::Function(vec![], Box::new(Type::Any)),
            IRValue::Null => Type::Null,
        }
    }
}

#[derive(Debug, Clone)]
pub enum IRConstant {
    Integer(i64),
    Float(f64),
    Boolean(bool),
    String(String),
    Array(Vec<IRConstant>),
    Struct(String, Vec<(String, IRConstant)>),
    Function(String, Vec<String>, Box<IRBlock>),
    Null,
}

impl From<IRConstant> for IRValue {
    fn from(constant: IRConstant) -> Self {
        match constant {
            IRConstant::Integer(i) => IRValue::Integer(i),
            IRConstant::Float(f) => IRValue::Float(f),
            IRConstant::Boolean(b) => IRValue::Boolean(b),
            IRConstant::String(s) => IRValue::String(s),
            IRConstant::Array(arr) => IRValue::Array(arr.into_iter().map(IRValue::from).collect()),
            IRConstant::Struct(name, fields) => IRValue::Struct(
                name,
                fields.into_iter().map(|(k, v)| (k, IRValue::from(v))).collect(),
            ),
            IRConstant::Function(name, params, body) => IRValue::Function(name, params, body),
            IRConstant::Null => IRValue::Null,
        }
    }
}

#[derive(Debug, Clone)]
pub struct IRBlock {
    pub instructions: Vec<IRInstruction>,
    pub locals: HashMap<String, Type>,
}

impl IRBlock {
    pub fn new() -> Self {
        Self {
            instructions: Vec::new(),
            locals: HashMap::new(),
        }
    }

    pub fn add_instruction(&mut self, instruction: IRInstruction) {
        self.instructions.push(instruction);
    }

    pub fn add_local(&mut self, name: String, ty: Type) {
        self.locals.insert(name, ty);
    }
}

#[derive(Debug, Clone)]
pub enum IRInstruction {
    Load(IRValue),
    Store(String, IRValue),
    Call(String, Vec<IRValue>),
    Return(Option<IRValue>),
    Jump(String),
    JumpIf(IRValue, String),
    Label(String),
} 