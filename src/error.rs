use std::fmt;

#[derive(Debug)]
pub enum SlangError {
    Syntax(String),
    Type(String),
    Runtime(String),
    IO(String),
}

impl fmt::Display for SlangError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            SlangError::Syntax(msg) => write!(f, "Syntax error: {}", msg),
            SlangError::Type(msg) => write!(f, "Type error: {}", msg),
            SlangError::Runtime(msg) => write!(f, "Runtime error: {}", msg),
            SlangError::IO(msg) => write!(f, "IO error: {}", msg),
        }
    }
}

impl std::error::Error for SlangError {}

pub type Result<T> = std::result::Result<T, SlangError>;

impl From<String> for SlangError {
    fn from(msg: String) -> Self {
        SlangError::Syntax(msg)
    }
}

impl From<std::io::Error> for SlangError {
    fn from(err: std::io::Error) -> Self {
        SlangError::IO(err.to_string())
    }
} 