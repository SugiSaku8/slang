pub mod lexer;
pub mod parser;
pub mod compiler;
pub mod runtime;
pub mod error;

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
} 