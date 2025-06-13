use logos::{Logos, Span};
use crate::error::Result;

#[derive(Logos, Debug, PartialEq, Clone)]
pub enum Token {
    // 識別子
    #[regex(r"[a-zA-Z_][a-zA-Z0-9_]*")]
    Identifier(String),

    // リテラル
    #[regex(r"[0-9]+")]
    IntegerLiteral(i64),

    #[regex(r"[0-9]+\.[0-9]+")]
    FloatLiteral(f64),

    #[regex(r#""([^"\\]|\\t|\\u|\\n|\\")*""#)]
    StringLiteral(String),

    #[regex(r"'([^'\\]|\\t|\\u|\\n|\\')'")]
    CharLiteral(char),

    // キーワード
    #[token("fn")]
    Fn,

    #[token("let")]
    Let,

    #[token("if")]
    If,

    #[token("else")]
    Else,

    #[token("match")]
    Match,

    #[token("while")]
    While,

    #[token("for")]
    For,

    #[token("return")]
    Return,

    // 演算子
    #[token("+")]
    Plus,

    #[token("-")]
    Minus,

    #[token("*")]
    Star,

    #[token("/")]
    Slash,

    #[token("=")]
    Equals,

    #[token("==")]
    EqualsEquals,

    #[token("!=")]
    NotEquals,

    #[token("<")]
    LessThan,

    #[token(">")]
    GreaterThan,

    #[token("<=")]
    LessEquals,

    #[token(">=")]
    GreaterEquals,

    // 区切り文字
    #[token("(")]
    LeftParen,

    #[token(")")]
    RightParen,

    #[token("{")]
    LeftBrace,

    #[token("}")]
    RightBrace,

    #[token("[")]
    LeftBracket,

    #[token("]")]
    RightBracket,

    #[token(";")]
    Semicolon,

    #[token(",")]
    Comma,

    #[token(".")]
    Dot,

    #[token("->")]
    Arrow,

    // 優先所有格関連
    #[token("Var:type:priority:")]
    VarTypePriority,

    #[token("Function:type:priority:")]
    FunctionTypePriority,

    #[token("Macro:type:")]
    MacroType,

    // エラー
    #[error]
    #[regex(r"[ \t\n\f]+", logos::skip)]
    Error,
}

pub struct Lexer<'a> {
    source: &'a str,
    tokens: Vec<(Token, Span)>,
    current: usize,
}

impl<'a> Lexer<'a> {
    pub fn new(source: &'a str) -> Result<Self> {
        let mut lex = logos::Lexer::<Token>::new(source);
        let mut tokens = Vec::new();

        while let Some(token) = lex.next() {
            match token {
                Ok(token) => tokens.push((token, lex.span())),
                Err(_) => return Err(crate::error::SlangError::Lexical(
                    format!("Invalid token at position {}", lex.span().start)
                )),
            }
        }

        Ok(Self {
            source,
            tokens,
            current: 0,
        })
    }

    pub fn peek(&self) -> Option<&Token> {
        self.tokens.get(self.current).map(|(token, _)| token)
    }

    pub fn next(&mut self) -> Option<&Token> {
        let token = self.peek();
        self.current += 1;
        token
    }

    pub fn span(&self) -> Option<Span> {
        self.tokens.get(self.current).map(|(_, span)| *span)
    }
} 