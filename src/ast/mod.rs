use crate::type_system::Type;
use std::fmt;

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
    pub params: Vec<Parameter>,
    pub return_type: Type,
    pub body: Block,
    pub priority: Option<FunctionPriority>,
}

#[derive(Debug, Clone)]
pub struct Parameter {
    pub name: String,
    pub type_: Type,
}

#[derive(Debug, Clone)]
pub struct Block {
    pub statements: Vec<Statement>,
}

#[derive(Debug, Clone)]
pub enum Statement {
    Let(LetStatement),
    Expression(Expression),
    Return(Option<Expression>),
    If(IfStatement),
    Match(MatchStatement),
    While(WhileStatement),
    For(ForStatement),
    Priority(PriorityStatement),
}

#[derive(Debug, Clone)]
pub struct LetStatement {
    pub name: String,
    pub type_: Option<Type>,
    pub value: Expression,
    pub priority: Option<MemoryPriority>,
}

#[derive(Debug, Clone)]
pub struct IfStatement {
    pub condition: Expression,
    pub then_block: Block,
    pub else_block: Option<Block>,
}

#[derive(Debug, Clone)]
pub struct MatchStatement {
    pub expression: Expression,
    pub arms: Vec<MatchArm>,
}

#[derive(Debug, Clone)]
pub struct MatchArm {
    pub pattern: Pattern,
    pub guard: Option<Expression>,
    pub body: Block,
}

#[derive(Debug, Clone)]
pub struct WhileStatement {
    pub condition: Expression,
    pub body: Block,
}

#[derive(Debug, Clone)]
pub struct ForStatement {
    pub pattern: Pattern,
    pub iterator: Expression,
    pub body: Block,
}

#[derive(Debug, Clone)]
pub struct PriorityStatement {
    pub priority_type: PriorityType,
    pub value: Expression,
}

#[derive(Debug, Clone)]
pub enum Expression {
    Literal(Literal),
    Identifier(String),
    Binary(BinaryExpression),
    Unary(UnaryExpression),
    Call(CallExpression),
    Member(MemberExpression),
    Index(IndexExpression),
    Block(Block),
    If(IfExpression),
    Match(MatchExpression),
    Lambda(LambdaExpression),
    Priority(PriorityExpression),
}

#[derive(Debug, Clone)]
pub enum Literal {
    Integer(i64),
    Float(f64),
    Boolean(bool),
    Character(char),
    String(String),
    Array(Vec<Expression>),
    Tuple(Vec<Expression>),
}

#[derive(Debug, Clone)]
pub struct BinaryExpression {
    pub left: Box<Expression>,
    pub operator: BinaryOperator,
    pub right: Box<Expression>,
}

#[derive(Debug, Clone)]
pub enum BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Equal,
    NotEqual,
    LessThan,
    GreaterThan,
    LessEqual,
    GreaterEqual,
    And,
    Or,
}

#[derive(Debug, Clone)]
pub struct UnaryExpression {
    pub operator: UnaryOperator,
    pub expression: Box<Expression>,
}

#[derive(Debug, Clone)]
pub enum UnaryOperator {
    Negate,
    Not,
}

#[derive(Debug, Clone)]
pub struct CallExpression {
    pub callee: Box<Expression>,
    pub arguments: Vec<Expression>,
}

#[derive(Debug, Clone)]
pub struct MemberExpression {
    pub object: Box<Expression>,
    pub property: String,
}

#[derive(Debug, Clone)]
pub struct IndexExpression {
    pub array: Box<Expression>,
    pub index: Box<Expression>,
}

#[derive(Debug, Clone)]
pub struct IfExpression {
    pub condition: Box<Expression>,
    pub then_branch: Box<Expression>,
    pub else_branch: Box<Expression>,
}

#[derive(Debug, Clone)]
pub struct MatchExpression {
    pub expression: Box<Expression>,
    pub arms: Vec<MatchArm>,
}

#[derive(Debug, Clone)]
pub struct LambdaExpression {
    pub params: Vec<Parameter>,
    pub return_type: Type,
    pub body: Box<Expression>,
}

#[derive(Debug, Clone)]
pub struct PriorityExpression {
    pub priority_type: PriorityType,
    pub expression: Box<Expression>,
}

#[derive(Debug, Clone)]
pub enum Pattern {
    Literal(Literal),
    Identifier(String),
    Tuple(Vec<Pattern>),
    Struct(String, Vec<(String, Pattern)>),
    Wildcard,
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