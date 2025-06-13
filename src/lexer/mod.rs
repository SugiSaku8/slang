use logos::{Logos, Span};
use std::ops::Range;

#[derive(Logos, Debug, PartialEq, Clone)]
pub enum Token {
    // 識別子
    #[regex(r"[a-zA-Z_][a-zA-Z0-9_]*", priority = 2)]
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
    Function,

    #[token("let")]
    Let,

    #[token("if")]
    If,

    #[token("else")]
    Else,

    #[token("while")]
    While,

    #[token("for")]
    For,

    #[token("in")]
    In,

    #[token("return")]
    Return,

    #[token("match")]
    Match,

    #[token("type")]
    Type,

    #[token("priority")]
    Priority,

    #[token("most_high")]
    MostHigh,

    #[token("true")]
    True,

    #[token("false")]
    False,

    #[token("null")]
    Null,

    // 演算子
    #[token("=")]
    Assign,

    #[token("==")]
    Equals,

    #[token("!=")]
    NotEquals,

    #[token("<")]
    LessThan,

    #[token(">")]
    GreaterThan,

    #[token("<=")]
    LessThanEquals,

    #[token(">=")]
    GreaterThanEquals,

    // 区切り文字
    #[token("(")]
    LParen,

    #[token(")")]
    RParen,

    #[token("{")]
    LBrace,

    #[token("}")]
    RBrace,

    #[token("[")]
    LBracket,

    #[token("]")]
    RBracket,

    #[token(":")]
    Colon,

    #[token(";")]
    Semicolon,

    #[token(",")]
    Comma,

    #[token(".")]
    Dot,

    #[token("->")]
    Arrow,

    #[token("=>")]
    FatArrow,

    #[token("_", priority = 3)]
    Underscore,

    // 優先所有格関連
    #[token("Var:type:priority:")]
    VarTypePriority,

    #[token("Function:type:priority:")]
    FunctionTypePriority,

    #[token("Macro:type:")]
    MacroType,

    #[regex(r"//[^\n]*")]
    Comment,

    #[regex(r"[ \t\n\f]+")]
    Whitespace,

    // 論理演算子
    #[token("&&")]
    And,

    #[token("||")]
    Or,

    #[token("!")]
    Not,

    #[token("%")]
    Percent,

    // エラー
    #[token("+")]
    Plus,

    #[token("-")]
    Minus,

    #[token("*")]
    Star,

    #[token("/")]
    Slash,
}

pub struct Lexer<'a> {
    source: &'a str,
    tokens: Vec<(Token, Range<usize>)>,
    current: usize,
}

impl<'a> Lexer<'a> {
    pub fn new(source: &'a str) -> Self {
        let mut lexer = Token::lexer(source);
        let mut tokens = Vec::new();
        while let Some(token) = lexer.next() {
            if let Ok(token) = token {
                if token != Token::Whitespace && token != Token::Comment {
                    tokens.push((token, lexer.span()));
                }
            }
        }
        Self {
            source,
            tokens,
            current: 0,
        }
    }

    pub fn peek(&self) -> Option<&Token> {
        self.tokens.get(self.current).map(|(token, _)| token)
    }

    pub fn next(&mut self) -> Option<&Token> {
        let current = self.current;
        if current >= self.tokens.len() {
            return None;
        }
        self.current += 1;
        Some(&self.tokens[current].0)
    }

    pub fn current_span(&self) -> Range<usize> {
        if let Some((_, span)) = self.tokens.get(self.current) {
            span.clone()
        } else {
            0..0
        }
    }
} 