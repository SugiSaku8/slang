use crate::type_system::Type;
use std::fmt;

#[derive(Debug, Clone)]
pub struct AST {
    pub functions: Vec<Function>,
    pub type_definitions: Vec<TypeDefinition>,
}

impl AST {
    pub fn new() -> Self {
        AST {
            functions: Vec::new(),
            type_definitions: Vec::new(),
        }
    }

    pub fn add_function(&mut self, function: Function) {
        self.functions.push(function);
    }

    pub fn add_type_definition(&mut self, type_def: TypeDefinition) {
        self.type_definitions.push(type_def);
    }
}

#[derive(Debug, Clone)]
pub struct Program {
    pub items: Vec<Item>,
}

#[derive(Debug, Clone)]
pub enum Item {
    Function(Function),
    Struct(Struct),
    Trait(Trait),
    Module(Module),
    Macro(Macro),
}

#[derive(Debug, Clone)]
pub struct Function {
    pub name: String,
    pub parameters: Vec<Parameter>,
    pub return_type: Type,
    pub priority: Vec<i32>,
    pub body: Vec<Statement>,
}

#[derive(Debug, Clone)]
pub struct Parameter {
    pub name: String,
    pub type_annotation: Type,
}

#[derive(Debug, Clone)]
pub struct TypeDefinition {
    pub name: String,
    pub type_: Type,
}

#[derive(Debug, Clone)]
pub enum Statement {
    Let(LetStatement),
    Return(Expression),
    If(IfStatement),
    While(WhileStatement),
    For(ForStatement),
    Match(MatchStatement),
    Expression(Expression),
}

#[derive(Debug, Clone)]
pub struct LetStatement {
    pub name: String,
    pub type_annotation: Option<Type>,
    pub value: Expression,
}

#[derive(Debug, Clone)]
pub struct IfStatement {
    pub condition: Expression,
    pub then_branch: Vec<Statement>,
    pub else_branch: Option<Vec<Statement>>,
}

#[derive(Debug, Clone)]
pub struct WhileStatement {
    pub condition: Expression,
    pub body: Vec<Statement>,
}

#[derive(Debug, Clone)]
pub struct ForStatement {
    pub variable: String,
    pub iterator: Expression,
    pub body: Vec<Statement>,
}

#[derive(Debug, Clone)]
pub struct MatchStatement {
    pub value: Expression,
    pub arms: Vec<MatchArm>,
}

#[derive(Debug, Clone)]
pub struct MatchArm {
    pub pattern: Pattern,
    pub body: Vec<Statement>,
}

#[derive(Debug, Clone)]
pub enum Pattern {
    Identifier(String),
    Tuple(Vec<Pattern>),
}

#[derive(Debug, Clone)]
pub enum Expression {
    Literal(Literal),
    Identifier(String),
    BinaryOp(BinaryOpExpression),
    UnaryOp(UnaryOpExpression),
    Call(CallExpression),
    Assignment(Box<AssignmentExpression>),
}

#[derive(Debug, Clone)]
pub enum Literal {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
}

#[derive(Debug, Clone)]
pub struct BinaryOpExpression {
    pub left: Box<Expression>,
    pub op: BinaryOperator,
    pub right: Box<Expression>,
}

#[derive(Debug, Clone)]
pub enum BinaryOperator {
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
pub struct UnaryOpExpression {
    pub op: UnaryOperator,
    pub right: Box<Expression>,
}

#[derive(Debug, Clone)]
pub enum UnaryOperator {
    Negate,
    Not,
}

#[derive(Debug, Clone)]
pub struct CallExpression {
    pub function: String,
    pub arguments: Vec<Expression>,
}

#[derive(Debug, Clone)]
pub struct AssignmentExpression {
    pub target: Expression,
    pub value: Expression,
}

#[derive(Debug, Clone)]
pub struct Struct {
    pub name: String,
    pub fields: Vec<StructField>,
}

#[derive(Debug, Clone)]
pub struct StructField {
    pub name: String,
    pub type_: Type,
    pub priority: Option<MemoryPriority>,
}

#[derive(Debug, Clone)]
pub struct Trait {
    pub name: String,
    pub methods: Vec<TraitMethod>,
}

#[derive(Debug, Clone)]
pub struct TraitMethod {
    pub name: String,
    pub params: Vec<Parameter>,
    pub return_type: Type,
}

#[derive(Debug, Clone)]
pub struct Module {
    pub name: String,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone)]
pub struct Macro {
    pub name: String,
    pub patterns: Vec<MacroPattern>,
    pub body: Block,
}

#[derive(Debug, Clone)]
pub struct MacroPattern {
    pub pattern: Pattern,
    pub guard: Option<Expression>,
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

impl fmt::Display for Program {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for item in &self.items {
            writeln!(f, "{}", item)?;
        }
        Ok(())
    }
}

impl fmt::Display for Item {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Item::Function(func) => write!(f, "{}", func),
            Item::Struct(struct_) => write!(f, "{}", struct_),
            Item::Trait(trait_) => write!(f, "{}", trait_),
            Item::Module(module) => write!(f, "{}", module),
            Item::Macro(macro_) => write!(f, "{}", macro_),
        }
    }
}

// 他のDisplay実装も同様に実装... 