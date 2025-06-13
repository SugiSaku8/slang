use crate::lexer::{Lexer, Token};
use crate::error::Result;
use std::fmt::Write;

#[derive(Debug, Clone, Copy)]
pub enum TokenKind {
    Keyword,
    Identifier,
    Literal,
    Operator,
    Delimiter,
    Comment,
    Priority,
}

#[derive(Debug, Clone)]
pub struct HighlightedToken {
    pub kind: TokenKind,
    pub text: String,
    pub span: (usize, usize),
}

pub struct Highlighter {
    tokens: Vec<HighlightedToken>,
}

impl Highlighter {
    pub fn new() -> Self {
        Self {
            tokens: Vec::new(),
        }
    }

    pub fn highlight(&mut self, source: &str) -> Result<String> {
        let lexer = Lexer::new(source)?;
        self.tokens.clear();

        for token in lexer.tokens {
            let (token, span) = token;
            let kind = self.get_token_kind(&token);
            let text = self.get_token_text(&token);
            self.tokens.push(HighlightedToken {
                kind,
                text,
                span,
            });
        }

        self.generate_highlighted_output()
    }

    fn get_token_kind(&self, token: &Token) -> TokenKind {
        match token {
            Token::Fn | Token::Let | Token::If | Token::Else | Token::Match |
            Token::While | Token::For | Token::Return => TokenKind::Keyword,
            Token::Identifier(_) => TokenKind::Identifier,
            Token::IntegerLiteral(_) | Token::FloatLiteral(_) | Token::StringLiteral(_) |
            Token::CharLiteral(_) => TokenKind::Literal,
            Token::Plus | Token::Minus | Token::Star | Token::Slash | Token::Equals |
            Token::EqualsEquals | Token::NotEquals | Token::LessThan | Token::GreaterThan |
            Token::LessEquals | Token::GreaterEquals => TokenKind::Operator,
            Token::LeftParen | Token::RightParen | Token::LeftBrace | Token::RightBrace |
            Token::LeftBracket | Token::RightBracket | Token::Semicolon | Token::Comma |
            Token::Dot | Token::Arrow => TokenKind::Delimiter,
            Token::VarTypePriority | Token::FunctionTypePriority | Token::MacroType => TokenKind::Priority,
            _ => TokenKind::Identifier,
        }
    }

    fn get_token_text(&self, token: &Token) -> String {
        match token {
            Token::Identifier(name) => name.clone(),
            Token::IntegerLiteral(value) => value.to_string(),
            Token::FloatLiteral(value) => value.to_string(),
            Token::StringLiteral(value) => format!("\"{}\"", value),
            Token::CharLiteral(value) => format!("'{}'", value),
            Token::Fn => "fn".to_string(),
            Token::Let => "let".to_string(),
            Token::If => "if".to_string(),
            Token::Else => "else".to_string(),
            Token::Match => "match".to_string(),
            Token::While => "while".to_string(),
            Token::For => "for".to_string(),
            Token::Return => "return".to_string(),
            Token::Plus => "+".to_string(),
            Token::Minus => "-".to_string(),
            Token::Star => "*".to_string(),
            Token::Slash => "/".to_string(),
            Token::Equals => "=".to_string(),
            Token::EqualsEquals => "==".to_string(),
            Token::NotEquals => "!=".to_string(),
            Token::LessThan => "<".to_string(),
            Token::GreaterThan => ">".to_string(),
            Token::LessEquals => "<=".to_string(),
            Token::GreaterEquals => ">=".to_string(),
            Token::LeftParen => "(".to_string(),
            Token::RightParen => ")".to_string(),
            Token::LeftBrace => "{".to_string(),
            Token::RightBrace => "}".to_string(),
            Token::LeftBracket => "[".to_string(),
            Token::RightBracket => "]".to_string(),
            Token::Semicolon => ";".to_string(),
            Token::Comma => ",".to_string(),
            Token::Dot => ".".to_string(),
            Token::Arrow => "->".to_string(),
            Token::VarTypePriority => "Var:type:priority:".to_string(),
            Token::FunctionTypePriority => "Function:type:priority:".to_string(),
            Token::MacroType => "Macro:type:".to_string(),
            Token::Error => "".to_string(),
        }
    }

    fn generate_highlighted_output(&self) -> Result<String> {
        let mut output = String::new();
        let mut current_pos = 0;

        for token in &self.tokens {
            // 空白を追加
            if token.span.0 > current_pos {
                output.push_str(&" ".repeat(token.span.0 - current_pos));
            }

            // トークンの種類に応じて色付け
            let colored_text = match token.kind {
                TokenKind::Keyword => format!("\x1b[1;34m{}\x1b[0m", token.text), // 青
                TokenKind::Identifier => format!("\x1b[1;36m{}\x1b[0m", token.text), // シアン
                TokenKind::Literal => format!("\x1b[1;33m{}\x1b[0m", token.text), // 黄
                TokenKind::Operator => format!("\x1b[1;35m{}\x1b[0m", token.text), // マゼンタ
                TokenKind::Delimiter => format!("\x1b[1;37m{}\x1b[0m", token.text), // 白
                TokenKind::Comment => format!("\x1b[1;32m{}\x1b[0m", token.text), // 緑
                TokenKind::Priority => format!("\x1b[1;31m{}\x1b[0m", token.text), // 赤
            };

            output.push_str(&colored_text);
            current_pos = token.span.1;
        }

        Ok(output)
    }
} 