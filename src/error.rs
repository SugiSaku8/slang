use thiserror::Error;

#[derive(Error, Debug)]
pub enum SlangError {
    #[error("Lexical error: {0}")]
    Lexical(String),

    #[error("Syntax error: {0}")]
    Syntax(String),

    #[error("Type error: {0}")]
    Type(String),

    #[error("Runtime error: {0}")]
    Runtime(String),

    #[error("Compilation error: {0}")]
    Compilation(String),
}

pub type Result<T> = std::result::Result<T, SlangError>; 