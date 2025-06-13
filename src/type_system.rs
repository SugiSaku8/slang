use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub enum Type {
    // 基本型
    Int,
    Float,
    Bool,
    Char,
    String,
    Void,

    // 複合型
    Array(Box<Type>, usize),
    Tuple(Vec<Type>),
    Struct(String, Vec<(String, Type)>),

    // 関数型
    Function(Vec<Type>, Box<Type>),

    // 数学的型
    Vector(usize, Box<Type>),
    Matrix(usize, usize, Box<Type>),
    Tensor(Vec<usize>, Box<Type>),
    Quaternion(Box<Type>),
    Complex(Box<Type>),

    // 優先所有格型
    PriorityType(Box<Type>, PriorityLevel),
}

#[derive(Debug, Clone, PartialEq)]
pub enum PriorityLevel {
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

impl fmt::Display for Type {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Type::Int => write!(f, "int"),
            Type::Float => write!(f, "float"),
            Type::Bool => write!(f, "bool"),
            Type::Char => write!(f, "char"),
            Type::String => write!(f, "string"),
            Type::Void => write!(f, "void"),
            Type::Array(t, size) => write!(f, "[{}; {}]", t, size),
            Type::Tuple(types) => {
                write!(f, "(")?;
                for (i, t) in types.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", t)?;
                }
                write!(f, ")")
            }
            Type::Struct(name, fields) => {
                write!(f, "struct {} {{ ", name)?;
                for (i, (name, t)) in fields.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}: {}", name, t)?;
                }
                write!(f, " }}")
            }
            Type::Function(params, ret) => {
                write!(f, "fn(")?;
                for (i, t) in params.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", t)?;
                }
                write!(f, ") -> {}", ret)
            }
            Type::Vector(size, t) => write!(f, "Vector<{}, {}>", size, t),
            Type::Matrix(rows, cols, t) => write!(f, "Matrix<{}, {}, {}>", rows, cols, t),
            Type::Tensor(dims, t) => {
                write!(f, "Tensor<[")?;
                for (i, dim) in dims.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", dim)?;
                }
                write!(f, "], {}>", t)
            }
            Type::Quaternion(t) => write!(f, "Quaternion<{}>", t),
            Type::Complex(t) => write!(f, "Complex<{}>", t),
            Type::PriorityType(t, p) => write!(f, "{} with priority {:?}", t, p),
        }
    }
} 