use crate::type_system::Type;
use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub struct AST {
    pub functions: Vec<Function>,
    pub type_definitions: Vec<TypeDefinition>,
}

impl AST {
    pub fn new() -> Self {
        Self {
            functions: Vec::new(),
            type_definitions: Vec::new(),
        }
    }

    pub fn add_function(&mut self, func: Function) {
        self.functions.push(func);
    }

    pub fn add_type_definition(&mut self, def: TypeDefinition) {
        self.type_definitions.push(def);
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Program {
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Item {
    Function(Function),
    Struct(Struct),
    Trait(Trait),
    Module(Module),
    Macro(Macro),
}

#[derive(Debug, Clone, PartialEq)]
pub struct Function {
    pub name: String,
    pub parameters: Vec<Parameter>,
    pub return_type: Type,
    pub priority: i32,
    pub body: Block,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Parameter {
    pub name: String,
    pub type_annotation: Type,
}

#[derive(Debug, Clone, PartialEq)]
pub struct TypeDefinition {
    pub name: String,
    pub fields: Vec<Field>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Field {
    pub name: String,
    pub type_annotation: Type,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Block {
    pub statements: Vec<Statement>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Statement {
    Let(LetStatement),
    Return(ReturnStatement),
    If(IfStatement),
    While(WhileStatement),
    For(ForStatement),
    Match(MatchStatement),
    Expression(Box<Expression>),
}

#[derive(Debug, Clone, PartialEq)]
pub struct LetStatement {
    pub name: String,
    pub type_annotation: Option<Type>,
    pub value: Box<Expression>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct ReturnStatement {
    pub value: Option<Box<Expression>>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct IfStatement {
    pub condition: Box<Expression>,
    pub then_block: Block,
    pub else_block: Option<Block>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct WhileStatement {
    pub condition: Box<Expression>,
    pub body: Block,
}

#[derive(Debug, Clone, PartialEq)]
pub struct ForStatement {
    pub variable: String,
    pub iterator: Box<Expression>,
    pub body: Block,
}

#[derive(Debug, Clone, PartialEq)]
pub struct MatchStatement {
    pub expression: Box<Expression>,
    pub arms: Vec<MatchArm>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct MatchArm {
    pub pattern: Pattern,
    pub body: Block,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Pattern {
    Identifier(String),
    Literal(Literal),
    Wildcard,
    Tuple(Vec<Pattern>),
    Struct { name: String, fields: Vec<FieldPattern> },
}

#[derive(Debug, Clone, PartialEq)]
pub struct FieldPattern {
    pub name: String,
    pub pattern: Box<Pattern>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Expression {
    Literal(Literal),
    Identifier(String),
    BinaryOp(Box<BinaryOpExpression>),
    UnaryOp(Box<UnaryOpExpression>),
    Call(Box<CallExpression>),
    Assignment(Box<AssignmentExpression>),
}

#[derive(Debug, Clone, PartialEq)]
pub enum Literal {
    Int(i64),
    Float(f64),
    Bool(bool),
    String(String),
    Null,
}

#[derive(Debug, Clone, PartialEq)]
pub struct BinaryOpExpression {
    pub left: Box<Expression>,
    pub op: BinaryOperator,
    pub right: Box<Expression>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum BinaryOperator {
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
    Equals,
    NotEquals,
    LessThan,
    GreaterThan,
    LessThanEquals,
    GreaterThanEquals,
    Divide,
    Modulo,
}

#[derive(Debug, Clone, PartialEq)]
pub struct UnaryOpExpression {
    pub op: UnaryOperator,
    pub right: Box<Expression>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum UnaryOperator {
    Neg,
    Not,
    Negate,
}

#[derive(Debug, Clone, PartialEq)]
pub struct CallExpression {
    pub function: String,
    pub arguments: Vec<Box<Expression>>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct AssignmentExpression {
    pub target: String,
    pub value: Box<Expression>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Struct {
    pub name: String,
    pub fields: Vec<StructField>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct StructField {
    pub name: String,
    pub type_: Type,
    pub priority: Option<MemoryPriority>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Trait {
    pub name: String,
    pub methods: Vec<TraitMethod>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct TraitMethod {
    pub name: String,
    pub params: Vec<Parameter>,
    pub return_type: Type,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Module {
    pub name: String,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Macro {
    pub name: String,
    pub patterns: Vec<MacroPattern>,
    pub body: Block,
}

#[derive(Debug, Clone, PartialEq)]
pub struct MacroPattern {
    pub pattern: Pattern,
    pub guard: Option<Expression>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum PriorityType {
    Log(LogPriority),
    Function(FunctionPriority),
    Memory(MemoryPriority),
}

#[derive(Debug, Clone, PartialEq)]
pub enum LogPriority {
    Log,
    Info,
    Debug,
    Warn,
    Alert,
    Error,
}

#[derive(Debug, Clone, PartialEq)]
pub enum FunctionPriority {
    Level(i32),
    MostLow,
    MostHigh,
}

#[derive(Debug, Clone, PartialEq)]
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

impl fmt::Display for AST {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for function in &self.functions {
            writeln!(f, "{}", function)?;
        }
        for type_def in &self.type_definitions {
            writeln!(f, "{}", type_def)?;
        }
        Ok(())
    }
}

impl fmt::Display for Function {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "fn {}({}) -> {} [priority: {}] {}", self.name, self.parameters.iter().map(|p| p.to_string()).collect::<Vec<_>>().join(", "), self.return_type, self.priority, self.body)
    }
}

impl fmt::Display for Parameter {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}: {}", self.name, self.type_annotation)
    }
}

impl fmt::Display for TypeDefinition {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "type {} {{ {} }}", self.name, self.fields.iter().map(|f| f.to_string()).collect::<Vec<_>>().join(", "))
    }
}

impl fmt::Display for Field {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}: {}", self.name, self.type_annotation)
    }
}

impl fmt::Display for Statement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Statement::Let(stmt) => write!(f, "{}", stmt),
            Statement::Return(stmt) => write!(f, "{}", stmt),
            Statement::If(stmt) => write!(f, "{}", stmt),
            Statement::While(stmt) => write!(f, "{}", stmt),
            Statement::For(stmt) => write!(f, "{}", stmt),
            Statement::Match(stmt) => write!(f, "{}", stmt),
            Statement::Expression(expr) => write!(f, "{}", expr),
        }
    }
}

impl fmt::Display for LetStatement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "let {}", self.name)?;
        if let Some(type_) = &self.type_annotation {
            write!(f, ": {}", type_)?;
        }
        writeln!(f, " = {};", self.value)
    }
}

impl fmt::Display for ReturnStatement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if let Some(value) = &self.value {
            writeln!(f, "return {};", value)
        } else {
            writeln!(f, "return;")
        }
    }
}

impl fmt::Display for IfStatement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "if {} {{", self.condition)?;
        for stmt in &self.then_block.statements {
            writeln!(f, "    {}", stmt)?;
        }
        if let Some(else_block) = &self.else_block {
            writeln!(f, "}} else {{")?;
            for stmt in &else_block.statements {
                writeln!(f, "    {}", stmt)?;
            }
        }
        writeln!(f, "}}")
    }
}

impl fmt::Display for WhileStatement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "while {} {{", self.condition)?;
        for stmt in &self.body.statements {
            writeln!(f, "    {}", stmt)?;
        }
        writeln!(f, "}}")
    }
}

impl fmt::Display for ForStatement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "for {} in {} {{", self.variable, self.iterator)?;
        for stmt in &self.body.statements {
            writeln!(f, "    {}", stmt)?;
        }
        writeln!(f, "}}")
    }
}

impl fmt::Display for MatchStatement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "match {} {{", self.expression)?;
        for arm in &self.arms {
            writeln!(f, "    {}", arm)?;
        }
        writeln!(f, "}}")
    }
}

impl fmt::Display for MatchArm {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "{} => {{", self.pattern)?;
        for stmt in &self.body.statements {
            writeln!(f, "        {}", stmt)?;
        }
        writeln!(f, "    }},")
    }
}

impl fmt::Display for Pattern {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Pattern::Literal(lit) => write!(f, "{}", lit),
            Pattern::Identifier(name) => write!(f, "{}", name),
            Pattern::Tuple(patterns) => {
                write!(f, "(")?;
                for (i, pattern) in patterns.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", pattern)?;
                }
                write!(f, ")")
            }
            Pattern::Struct { name, fields } => {
                write!(f, "{} {{", name)?;
                for (i, field) in fields.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", field)?;
                }
                write!(f, "}}")
            }
            Pattern::Wildcard => write!(f, "_"),
        }
    }
}

impl fmt::Display for FieldPattern {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}: {}", self.name, self.pattern)
    }
}

impl fmt::Display for Expression {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Expression::Literal(lit) => write!(f, "{}", lit),
            Expression::Identifier(name) => write!(f, "{}", name),
            Expression::BinaryOp(expr) => write!(f, "{}", expr),
            Expression::UnaryOp(expr) => write!(f, "{}", expr),
            Expression::Call(expr) => write!(f, "{}", expr),
            Expression::Assignment(expr) => write!(f, "{}", expr),
        }
    }
}

impl fmt::Display for BinaryOpExpression {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {} {}", self.left, self.op, self.right)
    }
}

impl fmt::Display for UnaryOpExpression {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}{}", self.op, self.right)
    }
}

impl fmt::Display for CallExpression {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}(", self.function)?;
        for (i, arg) in self.arguments.iter().enumerate() {
            if i > 0 {
                write!(f, ", ")?;
            }
            write!(f, "{}", arg)?;
        }
        write!(f, ")")
    }
}

impl fmt::Display for AssignmentExpression {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} = {}", self.target, self.value)
    }
}

impl fmt::Display for Literal {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Literal::Int(i) => write!(f, "{}", i),
            Literal::Float(fl) => write!(f, "{}", fl),
            Literal::Bool(b) => write!(f, "{}", b),
            Literal::String(s) => write!(f, "\"{}\"", s),
            Literal::Null => write!(f, "null"),
        }
    }
}

impl fmt::Display for BinaryOperator {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            BinaryOperator::Add => write!(f, "+"),
            BinaryOperator::Sub => write!(f, "-"),
            BinaryOperator::Mul => write!(f, "*"),
            BinaryOperator::Div => write!(f, "/"),
            BinaryOperator::Mod => write!(f, "%"),
            BinaryOperator::Eq => write!(f, "=="),
            BinaryOperator::Neq => write!(f, "!="),
            BinaryOperator::Lt => write!(f, "<"),
            BinaryOperator::Lte => write!(f, "<="),
            BinaryOperator::Gt => write!(f, ">"),
            BinaryOperator::Gte => write!(f, ">="),
            BinaryOperator::And => write!(f, "&&"),
            BinaryOperator::Or => write!(f, "||"),
            BinaryOperator::Equals => write!(f, "=="),
            BinaryOperator::NotEquals => write!(f, "!="),
            BinaryOperator::LessThan => write!(f, "<"),
            BinaryOperator::GreaterThan => write!(f, ">"),
            BinaryOperator::LessThanEquals => write!(f, "<="),
            BinaryOperator::GreaterThanEquals => write!(f, ">="),
            BinaryOperator::Divide => write!(f, "/"),
            BinaryOperator::Modulo => write!(f, "%"),
        }
    }
}

impl fmt::Display for UnaryOperator {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            UnaryOperator::Neg => write!(f, "-"),
            UnaryOperator::Not => write!(f, "!"),
            UnaryOperator::Negate => write!(f, "-"),
        }
    }
}

impl fmt::Display for Block {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{{\n")?;
        for stmt in &self.statements {
            writeln!(f, "  {}", stmt)?;
        }
        write!(f, "}}")
    }
}

// 他のDisplay実装も同様に実装... 