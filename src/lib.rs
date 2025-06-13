pub mod ast;
pub mod compiler;
pub mod error;
pub mod ir;
pub mod lexer;
pub mod parser;
pub mod runtime;
pub mod type_system;

pub use ast::*;
pub use compiler::*;
pub use error::*;
pub use ir::*;
pub use lexer::*;
pub use parser::*;
pub use runtime::*;
pub use type_system::*;

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
} 